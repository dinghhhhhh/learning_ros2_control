:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/doc/mock_components_userdoc.rst

.. _mock_components_userdoc:

Mock Components
----------------
Mock components 是对 hardware components（即 System、Sensor 与 Actuator）的简易“仿真”。
它们通过将 commands 镜像到对应的 states 来提供理想行为。
可在离线测试 ros2_control 框架时，用相应的 hardware interface 替代真实硬件。
主要优势是：即便没有真实硬件，也能测试框架内部的全部“配管”（piping）。
这意味着你可以测试 controllers、broadcaster、launch files，甚至与例如 MoveIt 的集成。
主要目的是减少在物理硬件上的调试时间，并加速开发。


Generic System
^^^^^^^^^^^^^^
该组件实现 ``hardware_interface::SystemInterface``，支持 command 与 state interfaces。
有关 hardware components 的更多信息，请参见 :ref:`详细文档 <overview_hardware_components>`。

特性：

  - 支持 mimic joints，并从 URDF 解析（参见 `URDF wiki <http://wiki.ros.org/urdf/XML/joint>`__）
  - 可将 commands 镜像到 states，支持带偏移与不带偏移
  - 假的 command interfaces，用于从外部节点设置 sensor 数据（可与 :ref:`forward controller <forward_command_controller_userdoc>` 配合）
  - 假的 gpio interfaces，用于从外部节点设置 sensor 数据（可与 :ref:`forward controller <forward_command_controller_userdoc>` 配合）


Parameters
,,,,,,,,,,

包含全部可选参数（及其默认值）的完整示例：

.. code-block:: xml

  <ros2_control name="MockHardwareSystem" type="system">
    <hardware>
      <plugin>mock_components/GenericSystem</plugin>
      <param name="calculate_dynamics">false</param>
      <param name="custom_interface_with_following_offset"></param>
      <param name="disable_commands">false</param>
      <param name="mock_gpio_commands">false</param>
      <param name="mock_sensor_commands">false</param>
      <param name="position_state_following_offset">0.0</param>
    </hardware>
    <joint name="joint1">
      <command_interface name="position"/>
      <command_interface name="velocity"/>
      <state_interface name="position">
        <param name="initial_value">3.45</param>
      </state_interface>
      <state_interface name="velocity"/>
      <state_interface name="acceleration"/>
    </joint>
    <joint name="joint2">
      <command_interface name="velocity"/>
      <command_interface name="acceleration"/>
      <state_interface name="position">
        <param name="initial_value">2.78</param>
      </state_interface>
      <state_interface name="position"/>
      <state_interface name="velocity"/>
      <state_interface name="acceleration"/>
    </joint>
    <gpio name="flange_vacuum">
      <command_interface name="vacuum"/>
      <state_interface name="vacuum" data_type="double"/>
    </gpio>
  </ros2_control>

使用 ``calculate_dynamics`` 的示例见 :ref:`example_2 <ros2_control_demos_example_2_userdoc>`；与 GPIO interfaces 组合使用见 :ref:`example_10 <ros2_control_demos_example_10_userdoc>`。

Component Parameters
####################

calculate_dynamics (optional; boolean; default: false)
  使用 Euler-forward 积分或有限差分，由 commands 计算 states。

custom_interface_with_following_offset (optional; string; default: "")
  将带偏移的 commands 映射到自定义 interface（参见 ``position_state_following_offset``）。

disable_commands (optional; boolean; default: false)
  禁用将 commands 镜像到 states。
  该选项有助于模拟与硬件的错误连接：一切未真正损坏，但突然某个 hardware interface 没有反馈。
  也可用于在硬件以开环（open loop）方式运行、没有反馈时测试你的配置。

mock_gpio_commands (optional; boolean; default: false)
  创建假的 command interfaces，以便用外部命令伪造 GPIO states。
  这些 interfaces 通常由 :ref:`forward controller <forward_command_controller_userdoc>` 使用，以提供从 ROS 侧的访问。

mock_sensor_commands (optional; boolean; default: false)
  创建假的 command interfaces，以便用外部命令伪造 sensor 测量值。
  这些 interfaces 通常由 :ref:`forward controller <forward_command_controller_userdoc>` 使用，以提供从 ROS 侧的访问。

position_state_following_offset (optional; double; default: 0.0)
  将 commands 镜像到 states 时，加到 state 值上的跟随偏移。
  若 ``custom_interface_with_following_offset`` 为空，偏移作用于 ``position`` state interface。
  若设置了自定义 interface，则将 ``position`` state 值 + 偏移应用到该 interface。

Per-Interface Parameters
########################

initial_value (optional; double)
  启动后某些 state interface 的初始值。示例：

  .. code-block:: xml

     <state_interface name="position">
       <param name="initial_value">3.45</param>
     </state_interface>

  若未设置，则在 ``configure`` lifecycle 转换时，joints 的 state interfaces 初始值设为 0.0。

  注意：该参数与 gz_ros2_control plugins 对 joint interfaces 共享。
  对于 Mock components，也可为 gpio 或 sensor 的 state interfaces 设置初始值。
