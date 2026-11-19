:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/doc/hardware_interface_types_userdoc.rst

.. _hardware_interface_types_userdoc:

``ros2_control`` hardware interface types
---------------------------------------------------------

``ros2_control`` 框架提供一组 hardware interface 类型，可用于为特定机器人或设备实现 hardware component。
以下各节描述不同的 hardware interface 类型及其用法。

Overview
*****************************
ros2_control 中的硬件以 URDF 描述，并在内部解析、封装为 ``HardwareInfo``。
定义见 `ros2_control 仓库 <https://github.com/ros-controls/ros2_control/blob/master/hardware_interface/include/hardware_interface/hardware_info.hpp>`_。
可查看其中定义的结构体，了解各 xml 标签可用的属性。
下方给出展示结构的通用示例。更具体的例子见下文 Example 部分。

.. code:: xml

    <ros2_control name="Name_of_the_hardware" type="system">
      <hardware>
        <plugin>library_name/ClassName</plugin>
        <!-- 加入 hardware_parameters -->
        <param name="example_param">value</param>
      </hardware>
      <joint name="name_of_the_component">
        <!-- `data_type` 参数可选（默认为 double）。 -->
        <command_interface name="interface_name" data_type="double">
          <!-- 以下均为可选。 -->
          <param name="min">-1</param>
          <param name="max">1</param>
          <param name="initial_value">0.0</param>
           <!-- 可选。加入键/值存储 parameters -->
          <param name="own_param_1">some_value</param>
          <param name="own_param_2">other_value</param>
        </command_interface>
        <!-- 定义 StateInterface 的简写形式。可像 CommandInterface 一样扩展。 -->
        <state_interface name="position"/>
      </joint>
    </ros2_control>

Joints
*****************************
``<joint>`` 标签将与物理机器人及 actuators 的 joints 相关联的 interfaces 分组。
它们具有 command 与 state interfaces，用于为硬件设定目标值并读取其当前状态。

在 ``<ros2_control>`` 标签中定义的所有 joints，都必须出现在 :ref:`controller manager 接收到的 URDF <doc/ros2_control/controller_manager/doc/userdoc:subscribers>` 中。

Joints 的 state interfaces 可通过 :ref:`joint_state_broadcaster <joint_state_broadcaster_userdoc>` 发布为 ROS topic。

Sensors
*****************************
``<sensor>`` 标签将多个 state interfaces 分组，用于描述例如硬件的内部状态。

根据 sensor 类型，ros2_controllers 中附带若干特定的 semantic components 及其 broadcasters，详见 :ref:`semantic_components <semantic_components>`。

GPIOs
*****************************
``<gpio>`` 标签用于描述无法关联到任何 joint 或 sensor 的机器人设备输入/输出端口。
``<gpio>`` 标签的解析方式与 ``<joint>`` 标签类似，具有 command 与 state interfaces。
该标签必须至少有一个 ``<command>`` 或 ``<state>`` 子标签。

选用 "gpio" 一词是因其通用性。
尽管严格意义上 gpio 用于数字信号，这里它可描述任何电气模拟量、数字信号或物理量。

``<gpio>`` 标签可作为全部三种 hardware component 类型（system、sensor 或 actuator）的子元素。

由于以 ``<gpio>`` 标签实现的端口通常高度应用相关，ros2_control 框架中不存在通用 publisher。
需要为每个应用实现自定义 gpio-controller。示例见 demo 仓库中的 :ref:`GPIO controller 示例 <ros2_control_demos_example_10_userdoc>`。

Hardware Groups
*****************************
Hardware Component Groups 是复杂系统中的关键组织机制，便于错误处理与容错。通过将相关 hardware components 分组（例如操作臂中的 actuators），用户可建立统一的错误检测与响应框架。

Hardware Component Groups 在互联 hardware components 之间传播错误方面发挥重要作用。例如，在操作臂系统中将 actuators 分组后，可实现错误传播：若组内某一 actuator 失败，错误可传播到其他 actuators，表明系统可能存在问题。默认情况下，actuator 错误仅隔离在其自身的 hardware component 内，其余部分可继续不受影响地运行。在给定的 ros2_control 配置中，每个 ``<ros2_control>`` 块内的 ``<group>`` 标签表示 hardware components 的分组，从而在系统中启用错误传播机制。

Data Types
*****************************
默认情况下，command 与 state interfaces 使用 ``double`` 数据类型。
但也可通过 ``<command_interface>`` 与 ``<state_interface>`` 标签中的可选 ``data_type`` 参数指定其他数据类型。
支持以下数据类型，以及未指定时的默认初始值：

* double（默认）：NaN
* float32：NaN
* bool：false
* uint8：255
* int8：127
* uint16：65535
* int16：32767
* uint32：4294967295
* int32：2147483647

Examples
*****************************
以下示例展示如何在 ``ros2_control`` URDF 中使用不同类型的 hardware interface。
它们可在不同的 hardware component 类型（system、actuator、sensor）中组合使用（:ref:`详见文档 <overview_hardware_components>`），示例如下。

1. 具有多个 GPIO interfaces 的机器人

   - RRBot System
   - Digital：4 路输入与 2 路输出（bool）
   - Analog：2 路输入与 1 路输出（uint16）
   - 法兰处的真空阀（bool）


  .. code:: xml

    <ros2_control name="RRBotSystemMutipleGPIOs" type="system">
      <hardware>
        <plugin>ros2_control_demo_hardware/RRBotSystemPositionOnlyHardware</plugin>
        <param name="example_param_hw_start_duration_sec">2.0</param>
        <param name="example_param_hw_stop_duration_sec">3.0</param>
        <param name="example_param_hw_slowdown">2.0</param>
      </hardware>
      <joint name="joint1">
        <command_interface name="position">
          <param name="min">-1</param>
          <param name="max">1</param>
        </command_interface>
        <state_interface name="position"/>
      </joint>
      <joint name="joint2">
        <command_interface name="position">
          <param name="min">-1</param>
          <param name="max">1</param>
        </command_interface>
        <state_interface name="position"/>
      </joint>
      <gpio name="flange_digital_IOs">
        <command_interface name="digital_output1" data_type="bool"/>
        <state_interface name="digital_output1" data_type="bool"/>    <!-- 需要以获知当前输出状态 -->
        <command_interface name="digital_output2" data_type="bool"/>
        <state_interface name="digital_output2" data_type="bool"/>
        <state_interface name="digital_input1" data_type="bool"/>
        <state_interface name="digital_input2" data_type="bool"/>
      </gpio>
      <gpio name="flange_analog_IOs">
        <command_interface name="analog_output1" data_type="uint16"/>
        <state_interface name="analog_output1" data_type="uint16">    <!-- 需要以获知当前输出状态 -->
          <param name="initial_value">3.1</param>  <!-- mock_hardware 的可选初始值 -->
        </state_interface>
        <state_interface name="analog_input1" data_type="uint16"/>
        <state_interface name="analog_input2" data_type="uint16"/>
      </gpio>
      <gpio name="flange_vacuum">
        <command_interface name="vacuum" data_type="bool"/>
        <state_interface name="vacuum" data_type="bool"/>    <!-- 需要以获知当前输出状态 -->
      </gpio>
    </ros2_control>

2. 兼具电气抓取与吸盘抓取能力的夹爪

   - 多模态夹爪
   - 1-DoF 平行夹爪
   - 吸盘开/关

  .. code:: xml

    <ros2_control name="MultimodalGripper" type="actuator">
      <hardware>
        <plugin>ros2_control_demo_hardware/MultimodalGripper</plugin>
      </hardware>
      <joint name="parallel_fingers">
        <command_interface name="position">
          <param name="min">0</param>
          <param name="max">100</param>
        </command_interface>
        <state_interface name="position"/>
      </joint>
      <gpio name="suction">
        <command_interface name="suction"/>
        <state_interface name="suction"/>    <!-- 需要以获知当前输出状态 -->
      </gpio>
    </ros2_control>

3. 带温度反馈与可调标定的力/力矩 Sensor

   - 2D FTS
   - 以 °C 为单位的温度反馈
   - 在 3 个标定矩阵（即标定量程）之间选择

  .. code:: xml

    <ros2_control name="RRBotForceTorqueSensor2D" type="sensor">
      <hardware>
        <plugin>ros2_control_demo_hardware/ForceTorqueSensor2DHardware</plugin>
        <param name="example_param_read_for_sec">0.43</param>
      </hardware>
      <sensor name="tcp_fts_sensor">
        <state_interface name="fx"/>
        <state_interface name="tz"/>
        <param name="frame_id">kuka_tcp</param>
        <param name="fx_range">100</param>
        <param name="tz_range">100</param>
      </sensor>
      <sensor name="temp_feedback">
        <state_interface name="temperature"/>
      </sensor>
      <gpio name="calibration">
        <command_interface name="calibration_matrix_nr"/>
        <state_interface name="calibration_matrix_nr"/>
      </gpio>
    </ros2_control>

4. 多个 hardware components 属于同一 group：``Group1``

   - RRBot System 1 与 2
   - Digital：共 4 路输入与 2 路输出
   - Analog：共 2 路输入与 1 路输出
   - 法兰处的真空阀（开/关）
   - Group：Group1

  .. code:: xml

    <ros2_control name="RRBotSystem1" type="system">
      <hardware>
        <plugin>ros2_control_demo_hardware/RRBotSystemPositionOnlyHardware</plugin>
        <group>Group1</group>
        <param name="example_param_hw_start_duration_sec">2.0</param>
        <param name="example_param_hw_stop_duration_sec">3.0</param>
        <param name="example_param_hw_slowdown">2.0</param>
      </hardware>
      <joint name="joint1">
        <command_interface name="position">
          <param name="min">-1</param>
          <param name="max">1</param>
        </command_interface>
        <state_interface name="position"/>
      </joint>
      <gpio name="flange_analog_IOs">
        <command_interface name="analog_output1"/>
        <state_interface name="analog_output1">    <!-- 需要以获知当前输出状态 -->
          <param name="initial_value">3.1</param>  <!-- mock_hardware 的可选初始值 -->
        </state_interface>
        <state_interface name="analog_input1"/>
        <state_interface name="analog_input2"/>
      </gpio>
      <gpio name="flange_vacuum">
        <command_interface name="vacuum"/>
        <state_interface name="vacuum"/>    <!-- 需要以获知当前输出状态 -->
      </gpio>
    </ros2_control>
    <ros2_control name="RRBotSystem2" type="system">
      <hardware>
        <plugin>ros2_control_demo_hardware/RRBotSystemPositionOnlyHardware</plugin>
        <group>Group1</group>
        <param name="example_param_hw_start_duration_sec">2.0</param>
        <param name="example_param_hw_stop_duration_sec">3.0</param>
        <param name="example_param_hw_slowdown">2.0</param>
      </hardware>
      <joint name="joint2">
        <command_interface name="position">
          <param name="min">-1</param>
          <param name="max">1</param>
        </command_interface>
        <state_interface name="position"/>
      </joint>
      <gpio name="flange_digital_IOs">
        <command_interface name="digital_output1"/>
        <state_interface name="digital_output1"/>    <!-- 需要以获知当前输出状态 -->
        <command_interface name="digital_output2"/>
        <state_interface name="digital_output2"/>
        <state_interface name="digital_input1"/>
        <state_interface name="digital_input2"/>
      </gpio>
    </ros2_control>
