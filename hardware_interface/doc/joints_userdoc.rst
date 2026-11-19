:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/doc/joints_userdoc.rst

.. _joints_userdoc:


Joint Kinematics for ros2_control
---------------------------------------------------------

本页概述 ros2_control 语境下的 joint kinematics。旨在对该主题做简要介绍，并说明当前在 ros2_control 中的实现。

Nomenclature
############

Degrees of Freedom (DoF)
  摘自 `wikipedia <https://en.wikipedia.org/wiki/Degrees_of_freedom_(mechanics)>`__：

    在物理学中，机械系统的自由度（DoF）是定义其构型或状态的独立参数个数。

Joint
  Joint 是两个 links 之间的连接。在 ROS 生态中，更常见的三类是：Revolute（带位置限位的铰链关节）、Continuous（无位置限位的连续铰链）或 Prismatic（沿轴线滑动的关节）。

  一般而言，joint 可以是驱动的（actuated）或非驱动的（也称为 passive）。Passive joints 没有自身的驱动机构，而是依靠外力或其他 joints 的被动带动来运动。一个 passive joint 可以有一个 DoF，例如摆锤；也可以是零 DoF 的并联机构的一部分。

Serial Kinematics
  Serial kinematics 指机器人操作臂中各 joints 彼此独立、且 joints 数量等于运动链 DoF 的布置方式。

  典型例子是具有六个 revolute joints、6-DoF 的工业机器人。每个 joint 可独立驱动，末端执行器可运动到工作空间中的任意位置与姿态。

Kinematic Loops
  另一方面，kinematic loops（也称 closed-loop mechanisms）涉及若干连接成运动链、并被一起驱动的 joints。这意味着这些 joints 是耦合的，不能独立运动：一般而言，DoF 数量小于 joints 数量。
  这种结构典型出现于并联机构，其中末端执行器通过若干运动链与基座相连。

  一个例子是四连杆机构，由四个 links 与四个 joints 组成。尽管有四个 joints，它可有一个或两个 actuators，因而具有一个或两个 DoFs。进一步可以说，我们有一个（两个）驱动 joint 与三个（两个）passive joints，后者必须满足机构的运动学约束。

URDF
#############

URDF 是 ROS 中描述机器人运动学的默认格式。但除所谓的 mimic joints 外，仅支持串联运动链。详见 `URDF specification <http://wiki.ros.org/urdf/XML/joint>`__。

Mimic joints 可在 URDF 中按如下方式定义：

  .. code-block:: xml

    <joint name="right_finger_joint" type="prismatic">
      <axis xyz="0 1 0"/>
      <origin xyz="0.0 -0.48 1" rpy="0.0 0.0 0.0"/>
      <parent link="base"/>
      <child link="finger_right"/>
      <limit effort="1000.0" lower="0" upper="0.38" velocity="10"/>
    </joint>
    <joint name="left_finger_joint" type="prismatic">
      <mimic joint="right_finger_joint" multiplier="1" offset="0"/>
      <axis xyz="0 1 0"/>
      <origin xyz="0.0 0.48 1" rpy="0.0 0.0 3.1415926535"/>
      <parent link="base"/>
      <child link="finger_left"/>
      <limit effort="1000.0" lower="0" upper="0.38" velocity="10"/>
    </joint>

Mimic joints 是对现实世界的一种抽象。例如，它们可用于描述：

* 关节位置与速度呈线性依赖的简单闭环运动学
* 用皮带连接的 links，如皮带轮系统或伸缩臂
* passive joints 的简化模型，例如末端执行器上始终朝下的摆锤
* 抽象复杂的驱动关节组，其中若干 joints 由底层控制环直接控制并同步运动。虽未给出现实例子，这可能是多个带有各自功率电子的电机，但使用相同设定值指令。

URDF 中定义的 mimic joints 由 Resource Manager 解析，并存入类型为 ``HardwareInfo`` 的类变量中，供 hardware components 访问。Mimic joints 不得具有 command interfaces，但可以具有 state interfaces。

.. code-block:: xml

  <ros2_control>
    <joint name="right_finger_joint">
      <command_interface name="effort"/>
      <state_interface name="position"/>
      <state_interface name="velocity"/>
      <state_interface name="effort"/>
    </joint>
    <joint name="left_finger_joint">
      <state_interface name="position"/>
      <state_interface name="velocity"/>
      <state_interface name="effort"/>
    </joint>
  </ros2_control>

在已正式发布的软件包中，已有如下包使用该信息：

* :ref:`mock_components (generic system) <mock_components_userdoc>`
* :ref:`gz_ros2_control <gz_ros2_control>`

由于 URDF 仅规定运动学，mimic 标签必须与 ros2_control 中使用的 hardware interface 类型无关。我们按如下方式解释该信息：

* **position = multiplier * other_joint_position + offset**
* **velocity = multiplier * other_joint_velocity**

若因故希望在不修改 URDF 的情况下停用 mimic joint 行为，可在 XML 的 ``<ros2_control>`` 段中为 joint 标签设置属性 ``mimic=false``。

.. code-block:: xml

  <joint name="left_finger_joint" mimic="false">
    <state_interface name="position"/>
    <state_interface name="velocity"/>
    <state_interface name="effort"/>
  </joint>

Transmission Interface
#######################
机械 transmission 对 effort/flow 变量进行变换，使其乘积（功率）保持恒定。线性和旋转域中的 effort 变量分别是力与力矩；flow 变量则分别是线速度与角速度。

在机器人学中，习惯上将 transmissions 置于 actuators 与 joints 之间。该 interface 沿用此命名，以标识变换的输入与输出空间。所提供的 interfaces 允许在 actuator 与 joint 空间之间对 effort、velocity 与 position 做双向映射。Position 不是功率变量，但可利用速度映射再加上表示 actuator 与 joint 零点偏移的积分常数来实现映射。

``transmission_interface`` 提供基类与若干 plugin 实现，可由自定义 hardware components 集成并加载。它们不会被任何 hardware component 或 gazebo plugins 自动加载；每个 hardware component 负责加载合适的 transmission interface，以将 actuator 读数映射为 joint 读数。

当前可用的实现有：

* ``SimpleTransmission``：具有恒定减速比、无额外动力学的简单 transmission。
* ``DifferentialTransmission``：具有两个 actuators 与两个 joints 的差动 transmission。
* ``FourBarLinkageTransmission``：具有两个 actuators 与两个 joints 的四连杆 transmission。

更多信息见 :ref:`example_8 <ros2_control_demos_example_8_userdoc>` 或 `transmission_interface <http://docs.ros.org/en/{DISTRO}/p/transmission_interface/index.html>`__ 文档。

Simulating Closed-Loop Kinematic Chains
#######################################
根据所用仿真 plugin 的不同，可采用不同方法来仿真闭环运动链。以下列表概述可用仿真 plugins 及其能力：

gazebo_ros2_control:
  * mimic joints
  * 闭环运动学可通过 URDF 中的 ``<gazebo>`` 标签支持，例如参见 `此处 <http://classic.gazebosim.org/tutorials?tut=kinematic_loop>`__。

gz_ros2_control:
  * mimic joints
  * 闭环运动学尚不直接支持，但可通过自定义 plugins 使用 ``DetachableJoint`` 实现。关注该主题的进展可跟踪 `此 issue <https://github.com/gazebosim/gz-physics/issues/25>`__。
