:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/doc/different_update_rates_userdoc.rst

.. _different_update_rates_userdoc:

Different update rates for Hardware Components
===============================================================================

``ros2_control`` 框架允许不同 hardware components 以不同的更新频率运行。当某些 hardware components 需要以不同于传统控制循环频率（即与 ``controller_manager`` 相同的频率）运行时，这一功能很有用。在包含不同 sensors、或使用不同通信协议的机器人系统中，各组件采用不同频率非常常见。
例如，某个 hardware component 需要比其余组件更高的频率运行：你可能需要以 1000Hz 读取某个 sensor，而其余组件以 500Hz 运行，前提是 ``controller_manager`` 的控制循环频率高于 1000Hz。只需在 hardware component 的 ``ros2_control`` 标签上添加参数 ``rw_rate``，即可方便地定义 read/write 频率。

Examples
*****************************
以下示例展示如何在 ``ros2_control`` URDF 中为不同类型的 hardware interface 使用不同更新频率。
它们可在不同的 hardware component 类型（system、actuator、sensor）中组合使用（:ref:`详见文档 <overview_hardware_components>`），示例如下。

带多模态夹爪与外部 sensor、且以不同频率运行的 RRBot：

.. code-block:: xml

  <ros2_control name="RRBotSystemMutipleGPIOs" type="system" rw_rate="500">
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
      <command_interface name="digital_output1"/>
      <state_interface name="digital_output1"/>    <!-- 需要以获知当前输出状态 -->
      <command_interface name="digital_output2"/>
      <state_interface name="digital_output2"/>
      <state_interface name="digital_input1"/>
      <state_interface name="digital_input2"/>
    </gpio>
  </ros2_control>
  <ros2_control name="MultimodalGripper" type="actuator" rw_rate="200">
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
  <ros2_control name="RRBotForceTorqueSensor2D" type="sensor" rw_rate="250">
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

在上述示例中，控制 RRBot joints 的 system hardware component 以 500 Hz 运行，多模态夹爪以 200 Hz 运行，力/力矩 sensor 以 250 Hz 运行。

.. note::
  上述示例中，system、actuator 与 sensor 三类 hardware components 的 ``rw_rate`` 分别设为 500 Hz、200 Hz 与 250 Hz。该参数为可选；若未设置，默认值为 0，表示该 hardware component 以与 ``controller_manager`` 相同的频率运行。但若指定频率高于 ``controller_manager`` 的频率，则该 hardware component 将按 ``controller_manager`` 的频率运行。
