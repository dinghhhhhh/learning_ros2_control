:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/doc/introspection.rst

ros2_control 配置的 introspection
***************************************

集成 ``pal_statistics`` 包后，``controller_manager`` 节点会将同一进程内已注册的变量发布到 ``~/introspection_data`` topic。
默认情况下，``controller_manager`` 中所有 ``State`` 与 ``Command`` interface 在添加时注册，从 ``ResourceManager`` 移除时注销。
所有已注册实体的状态在 ``controller_manager`` 每次 ``update`` 周期结束时发布。例如，在完整的同步 ros2_control 配置（同步 controller 与 Hardware Component）中，``Command`` interface 中的数据即为 Hardware Component 用于驱动硬件的命令。

所有已注册变量通过 3 个 topic 发布：``~/introspection_data/full``、``~/introspection_data/names`` 和 ``~/introspection_data/values``。
- ``~/introspection_data/full`` topic 在单条消息中发布包含名称与取值的完整 introspection 数据。便于从命令行跟踪或查看变量与信息。
- ``~/introspection_data/names`` topic 发布已注册变量的名称。仅在变量注册或注销时发布。
- ``~/introspection_data/values`` topic 发布已注册变量的取值。

只要至少有一个订阅者，``~/introspection_data/full`` 与 ``~/introspection_data/values`` 会在每个 update 周期异步发布。

``~/introspection_data/full`` 可与自定义可视化工具集成，或从命令行跟踪变量。``~/introspection_data/names`` 与 ``~/introspection_data/values`` 供 `PlotJuggler <https://github.com/PlotJuggler/PlotJuggler>`_ 或 `RQT plot <http://wiki.ros.org/rqt_plot>`_ 等可视化工具使用。

.. note::
  若数据频率很高，建议使用 ``~/introspection_data/names`` 与 ``~/introspection_data/values`` topic，以减少传输与存储的数据量。

.. _monitoring_and_tuning:

除上述 introspection 数据外，``controller_manager`` 还会发布 Hardware Component 的 read/write 周期以及 controller 的 update 周期的执行时间与周期性统计。通过注册这些变量的 statistics 并发布到 ``~/statistics`` topic 实现，摘要也会发布到 ``/diagnostics`` topic。

所有已注册变量通过 3 个 topic 发布：``~/statistics/full``、``~/statistics/names`` 和 ``~/statistics/values``。
- ``~/statistics/full`` topic 在单条消息中发布包含名称与取值的完整 introspection 数据。便于从命令行跟踪或查看变量与信息。
- ``~/statistics/names`` topic 发布已注册变量的名称。仅在变量注册或注销时发布。
- ``~/statistics/values`` topic 发布已注册变量的取值。

该 topic 主要用于检视 realtime 循环的行为。对须满足严格截止时间的硬件，以及理解生态中哪一部分在 realtime 循环中耗时更多，都非常关键。

如何对 controller 与 Hardware Component 的内部变量进行 introspection
============================================================================

可将 controller 或 Hardware Component 的任意成员变量注册用于 introspection。务必保证该变量的生命周期与 controller 或 Hardware Component 存续期一致。

.. note::
  若变量生命周期管理不当，可能被尝试读取，最坏情况下会导致 segmentation fault。

如何注册变量用于 introspection
---------------------------------------------

1. 在 controller 或 Hardware Component 头文件中包含必要头文件。

   .. code-block:: cpp

     #include <hardware_interface/introspection.hpp>

2. 在 controller 或 Hardware Component 的 configure 方法中注册变量。

   .. code-block:: cpp

     void MyController::on_configure()
     {
       ...
       // 注册变量用于 introspection（默认关闭）
       // 仅在 controller 处于 active 时进行 introspection，
       // 在 controller deactivate 时关闭。
       REGISTER_ROS2_CONTROL_INTROSPECTION("my_variable_name", &my_variable_);
       ...
     }

3. 默认情况下，所有已注册变量的 introspection 仅在 controller 与 Hardware Component 处于 active 时启用，在 deactivate 时关闭。

   .. note::
      若希望在 controller 或 Hardware Component 非 active 时仍保持 introspection，可在注册变量后于 ``on_configure`` 与 ``on_deactivate`` 中调用 ``this->enable_introspection(true)``。

可 introspection 的实体类型
-------------------------------------------

- 可转换为 double 的任意变量均可注册。
- 返回可转换为 double 的取值的函数也可注册。
- 复杂结构体的变量可为其每个内部变量定义 introspection 后注册。
- 自定义类型可通过定义 `custom introspection function <https://github.com/pal-robotics/pal_statistics/blob/humble-devel/pal_statistics/include/pal_statistics/registration_utils.hpp>`_ 进行 introspection。

  .. note::
    注册变量用于 introspection 不是 realtime 安全的。建议仅在 ``on_configure`` 方法中注册。


数据可视化
*******************

可使用任意显示 ROS topic 的工具进行可视化，但我们推荐 `PlotJuggler <https://github.com/PlotJuggler/PlotJuggler>`_ 查看高分辨率实时数据或 bag 中的数据。

1. 在命令行运行 ``ros2 run plotjuggler plotjuggler`` 打开 ``PlotJuggler``。

   .. image:: images/plotjuggler.png
      :alt: PlotJuggler

2. 从 ros2bag 文件导入数据，或在 ``Streaming`` 标题下使用 ``ROS2 Topic Subscriber`` 选项订阅 ROS2 topic 进行可视化。

3. 在弹出窗口中选择 ``~/introspection_data/names`` 与 ``~/introspection_data/values`` topic。

   .. image:: images/plotjuggler_select_topics.png
      :alt: PlotJuggler Select Topics

4. 选择感兴趣的变量并拖到图表中。

   .. image:: images/plotjuggler_visualizing_data.png
      :alt: PlotJuggler Visualizing Data