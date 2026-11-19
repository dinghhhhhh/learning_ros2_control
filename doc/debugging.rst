:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/doc/debugging.rst

调试
^^^^^^^^^

所有 controller 与 Hardware Component 均以 plugin 形式加载到 ``controller_manager`` 中。因此，调试器必须附加到 ``controller_manager`` 进程。若机器人或设备上运行多个 ``controller_manager`` 实例，需附加到与待调试 Hardware Component 或 controller 相关联的那个 ``controller_manager``。

操作指南
******************

* 在系统上安装 ``xterm``、``gdb`` 和 ``gdbserver``

  .. code-block:: bash

    sudo apt install xterm gdb gdbserver

* 确保使用 “debug” 或 “release with debug information” 构建：
  向 ``colcon build`` 传入 ``--cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo`` 即可。
  请注意，在 release 构建中，部分断点可能不如预期，因为对应行可能已被编译器优化。此类情况建议使用完整 Debug 构建（``--cmake-args -DCMAKE_BUILD_TYPE=Debug``）。

* 修改 launch 文件，使 controller manager 在附加调试器的情况下运行：

  * 方案 A：直接用 gdb CLI 运行：

    在 launch 文件的 ``controller_manager`` 节点项中添加 ``prefix=['xterm -e gdb -ex run --args']``。
    由于 ``ros2launch`` 的工作方式，需要在单独的终端实例中运行该节点。

  * 方案 B：使用 gdbserver 运行：

    在 launch 文件的 ``controller_manager`` 节点项中添加 ``prefix=['gdbserver localhost:3000']``。
    之后可附加 gdb CLI 或任意 IDE 到该 ``gdbserver`` 实例。
    请从已 source 工作空间的终端启动调试器，以便正确解析所有路径。

  launch 文件节点示例：

  .. code-block:: python

    # 获取 ros2 control 节点的 controller 配置文件
    controller_config_file = get_package_file("<package name>", "config/controllers.yaml")

    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[controller_config_file],
        output="both",
        emulate_tty=True,
        remappings=[
            ("~/robot_description", "/robot_description")
        ],
        prefix=['xterm -e gdb -ex run --args']  # or prefix=['gdbserver localhost:3000']
    )

    ld.add_action(controller_manager)

捕获异常
************************

* controller manager 默认会捕获 controller 与 Hardware Component 抛出的大部分异常，以避免整个系统崩溃。不过它仍会将异常类型与消息打印到控制台。
  这会使调试变难：调试器可能无法捕获异常，且异常消息不够清晰时也难以定位根因。
  可将 controller manager 节点中的 parameter ``handle_exceptions`` 设为 ``false`` 以关闭该行为，这样异常会向上传播到 controller manager，既可被调试器捕获，也会在正常执行时通过堆栈跟踪崩溃。

  controller manager 配置示例：

  .. code-block:: yaml

    controller_manager:
      ros__parameters:
        update_rate: 1000
        handle_exceptions: false

补充说明
*****************

* 调试 plugin

  只能在 plugin 加载之后在其内部设置断点。在 ros2_control 场景下，即需在 controller / Hardware Component 加载之后：

* Debug 构建

  通常只为要调试的特定包加入调试信息更实用。
  ``colcon build --packages-select [package_name] --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo`` 或 ``colcon build --packages-select [package_name] --cmake-args -DCMAKE_BUILD_TYPE=Debug``

* realtime

  .. warning::

    controller 的 ``update/on_activate/on_deactivate`` 方法以及 Hardware Component 的 ``read/write/on_activate/perform_command_mode_switch`` 方法均在 realtime update 循环上下文中运行。在这些位置设置断点可能导致问题，最坏情况下甚至可能损坏硬件。

  根据经验，在 realtime 上下文中更适合使用有意义的日志（需谨慎），或增加额外的 debug StateInterface（对 controller 而言可使用 publisher）。

不过，用 gdb 运行 controller_manager 与 plugin 对调试段错误等问题仍很有用，因为可以获取完整 backtrace。

参考资料
***********

* `ROS 2 and GDB <https://juraph.com/miscellaneous/ros2_and_gdb/>`_
* `Using GDB to debug a plugin <https://stackoverflow.com/questions/10919832/how-to-use-gdb-to-debug-a-plugin>`_
* `GDB CLI Tutorial <https://sourceware.org/gdb/onlinedocs/refcard.pdf>`_