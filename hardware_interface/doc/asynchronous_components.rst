:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/doc/asynchronous_components.rst

.. _asynchronous_components:

Running Hardware Components Asynchronously
============================================

``ros2_control`` 框架支持异步运行 hardware components。当某些 hardware components 需要在独立线程或 executor 中运行时，这一功能很有用。例如，某个 sensor 读取数据耗时较长，会影响 ``controller_manager`` 控制循环的周期性。此时可将该 sensor 放到独立线程或 executor 中运行，以避免阻塞控制循环。

Parameters
-----------

可在 ``ros2_control`` 硬件配置中设置以下参数，以使 hardware component 异步运行：

* ``is_async``：（可选）设为 ``true`` 时，hardware component 将异步运行。默认值为 ``false``。

在 ``ros2_control`` 标签下，可添加 ``properties`` 标签，以指定异步 hardware component 的如下参数：

* ``thread_priority``：（可选）运行该 hardware component 的线程优先级。取值为 0 到 99 的整数，默认值为 50。
* ``affinity``：（可选）运行该 hardware component 的线程的 CPU affinity。取值为 CPU 核心 ID 列表。默认值为空列表，表示线程可在任意 CPU 核心上运行。
* ``scheduling_policy``：（可选）运行该 hardware component 的线程的调度策略，可为以下取值之一：

  * ``synchronized`` (默认)：线程与主 controller_manager 线程同步运行。由 controller_manager 负责触发该 hardware component 的 read 与 write 调用。
  * ``detached``：线程独立于主 controller_manager 线程运行。由 hardware component 自行管理触发 read 与 write 调用的时序。

* ``print_warnings``：（可选）若设为 ``true``，当线程无法满足时序要求时将打印警告。默认值为 ``true``。

.. note::
  线程优先级仅在 hardware component 异步运行时使用。
  当 hardware component 异步运行时，使用 FIFO 调度策略。

Examples
---------

以下示例展示如何在 ``ros2_control`` URDF 中同步或异步使用不同类型的 hardware interface。
它们可在不同的 hardware component 类型（system、actuator、sensor）中组合使用（:ref:`详见文档 <overview_hardware_components>`），示例如下。

带多模态夹爪与外部 sensor 的 RRBot：

.. code-block:: xml

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
      <command_interface name="digital_output1"/>
      <state_interface name="digital_output1"/>    <!-- 需要以获知当前输出状态 -->
      <command_interface name="digital_output2"/>
      <state_interface name="digital_output2"/>
      <state_interface name="digital_input1"/>
      <state_interface name="digital_input2"/>
    </gpio>
  </ros2_control>
  <ros2_control name="MultimodalGripper" type="actuator" is_async="true">
    <properties>
      <async affinity="[2,4]" scheduling_policy="synchronized" print_warnings="true" thread_priority="30"/>
    </properties>
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
  </ros2_control>
  <ros2_control name="RRBotForceTorqueSensor2D" type="sensor" is_async="true">
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

上述示例中定义了如下组件：

* 名为 ``RRBotSystemMutipleGPIOs`` 的 system hardware component，包含两个 joints 与一个 GPIO component，同步运行。
* 名为 ``MultimodalGripper`` 的 actuator hardware component，包含一个 joint，异步运行，线程优先级为 30。
* 名为 ``RRBotForceTorqueSensor2D`` 的 sensor hardware component，包含两个 sensors 与一个 GPIO component，异步运行，使用默认线程优先级 50。
