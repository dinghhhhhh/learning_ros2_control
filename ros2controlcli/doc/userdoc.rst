:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/ros2controlcli/doc/userdoc.rst

.. _ros2controlcli_userdoc:

命令行界面
======================

下列命令支持从命令行与 ``controller_manager`` 交互。它们通过标准 ROS2 CLI 框架提供。

当前支持的命令包括

    - ros2 control list_controllers
    - ros2 control list_controller_types
    - ros2 control list_hardware_components
    - ros2 control list_hardware_interfaces
    - ros2 control load_controller
    - ros2 control reload_controller_libraries
    - ros2 control set_controller_state
    - ros2 control set_hardware_component_state
    - ros2 control switch_controllers
    - ros2 control unload_controller
    - ros2 control cleanup_controller
    - ros2 control view_controller_chains
    - ros2 control view_hardware_status


list_controllers
----------------

.. code-block:: console

    $ ros2 control list_controllers -h
    usage: ros2 control list_controllers [-h] [--spin-time SPIN_TIME] [-s] [--claimed-interfaces] [--required-state-interfaces] [--required-command-interfaces] [--chained-interfaces] [--exported-interfaces] [--verbose] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...]

    输出已加载 controller 的列表及其类型与状态

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      --claimed-interfaces  列出 controller 已声明（claimed）的 interface
      --required-state-interfaces
                            列出 controller 所需的 state interface
      --required-command-interfaces
                            列出 controller 所需的 command interface
      --chained-interfaces  列出 controller 链接（chain）到的 interface
      --exported-interfaces
                            列出 controller 导出的 state 与 reference interface
      --verbose, -v         列出 controller 已声明的 interface、所需的 state interface 与所需的 command interface
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数

输出示例:

.. code-block:: console

    $ ros2 control list_controllers
    test_controller_name[test_controller]    active


list_controller_types
---------------------

.. code-block:: console

    $ ros2 control list_controller_types -h
    usage: ros2 control list_controller_types [-h] [--spin-time SPIN_TIME] [-s] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...]

    输出可用的 controller 类型及其基类

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数

输出示例:

.. code-block:: console

    $ ros2 control list_controller_types
    diff_drive_controller/DiffDriveController                              controller_interface::ControllerInterface
    joint_state_broadcaster/JointStateBroadcaster                          controller_interface::ControllerInterface
    joint_trajectory_controller/JointTrajectoryController                  controller_interface::ControllerInterface


list_hardware_components
------------------------

.. code-block:: console

    $ ros2 control list_hardware_components -h
    usage: ros2 control list_hardware_components [-h] [--spin-time SPIN_TIME] [-s] [--verbose] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...]

    输出可用的 hardware component 列表

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      --verbose, -v         列出 hardware component 及其 command 与 state interface 及数据类型
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数

输出示例:

.. code-block:: console

    $ ros2 control list_hardware_components
    Hardware Component 0
        name: RRBot
        type: system
        plugin name: ros2_control_demo_hardware/RRBotSystemPositionOnlyHardware
        state: id=3 label=active
        command interfaces
          joint2/position [available] [claimed]
          joint1/position [available] [claimed]


.. code-block:: console

    $ ros2 control list_hardware_components -v
    Hardware Component 0
        name: RRBot
        type: system
        plugin name: ros2_control_demo_hardware/RRBotSystemPositionOnlyHardware
        state: id=3 label=active
        command interfaces
          joint2/position [double] [available] [claimed]
          joint1/position [double] [available] [claimed]
        state interfaces
          joint2/position [double] [available]
          joint1/position [double] [available]


list_hardware_interfaces
------------------------

.. code-block:: console

    $ ros2 control list_hardware_interfaces -h
    usage: ros2 control list_hardware_interfaces [-h] [--spin-time SPIN_TIME] [-s] [--verbose] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...]

    输出可用的 command 与 state interface 列表

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      --verbose, -v         列出 hardware interface 及其数据类型
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数


.. code-block:: console

    $ ros2 control list_hardware_interfaces
    command interfaces
      joint1/position [unclaimed]
      joint2/position [unclaimed]
    state interfaces
      joint1/position
      joint2/position

.. code-block:: console

    $ ros2 control list_hardware_interfaces -v
    command interfaces
      joint1/position [double] [unclaimed]
      joint2/position [double] [unclaimed]
    state interfaces
      joint1/position [double]
      joint2/position [double]


load_controller
---------------

.. code-block:: console

    $ ros2 control load_controller -h
    usage: ros2 control load_controller [-h] [--spin-time SPIN_TIME] [-s] [--set-state {inactive,active}] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...] controller_name [param_file]

    在 controller manager 中加载 controller

    位置参数:
      controller_name       controller 的名称
      param_file            包含 controller parameter 的 YAML 文件

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      --set-state {inactive,active}
                            设置所加载 controller 的状态
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数

reload_controller_libraries
---------------------------

.. code-block:: console

    $ ros2 control reload_controller_libraries -h
    usage: ros2 control reload_controller_libraries [-h] [--spin-time SPIN_TIME] [-s] [--force-kill] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...]

    重新加载 controller 库

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      --force-kill          强制停止已加载的 controller
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数

set_controller_state
--------------------

.. code-block:: console

    $ ros2 control set_controller_state -h
    usage: ros2 control set_controller_state [-h] [--spin-time SPIN_TIME] [-s] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...] controller_name {unconfigured,inactive,active}

    调整 controller 的状态

    位置参数:
      controller_name       要更改的 controller 名称
      {unconfigured,inactive,active}     要将 controller 切换到的目标状态

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数

set_hardware_component_state
----------------------------

.. code-block:: console

    $ ros2 control set_hardware_component_state -h
    usage: ros2 control set_hardware_component_state [-h] [--spin-time SPIN_TIME] [-s] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...] hardware_component_name {unconfigured,inactive,active}

    调整 hardware component 的状态

    位置参数:
      hardware_component_name
                            要更改的 hardware component 名称
      {unconfigured,inactive,active}
                            要将 hardware component 切换到的目标状态

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数

switch_controllers
------------------

.. code-block:: console

    $ ros2 control switch_controllers -h
    usage: ros2 control switch_controllers [-h] [--spin-time SPIN_TIME] [-s] [--deactivate [DEACTIVATE ...]] [--activate [ACTIVATE ...]] [--strict] [--activate-asap] [--switch-timeout SWITCH_TIMEOUT] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...]

    在 controller manager 中切换 controller

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      --deactivate [DEACTIVATE ...]
                            要停用的 controller 名称
      --activate [ACTIVATE ...]
                            要激活的 controller 名称
      --strict              严格切换
      --activate-asap       尽快启动 controller
      --switch-timeout SWITCH_TIMEOUT
                            切换 controller 的超时时间
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数

unload_controller
-----------------

.. code-block:: console

    $ ros2 control unload_controller -h
    usage: ros2 control unload_controller [-h] [--spin-time SPIN_TIME] [-s] [--all-inactive] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...] [controller_name]

    在 controller manager 中卸载 controller

    位置参数:
      controller_name       controller 的名称

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      --all-inactive        卸载当前处于 ``inactive`` 状态的全部 controller
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数

必须指定 ``controller_name`` 或 ``--all-inactive`` 之一，但不能同时指定两者。

卸载单个 controller 的示例:

.. code-block:: console

    $ ros2 control unload_controller test_controller_name

卸载当前全部 inactive controller 的示例:

.. code-block:: console

    $ ros2 control unload_controller --all-inactive

cleanup_controller
------------------

.. code-block:: console

    $ ros2 control cleanup_controller -h
    usage: ros2 control cleanup_controller [-h] [--spin-time SPIN_TIME] [-s] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...] controller_name

    在 controller manager 中清理 controller

    位置参数:
      controller_name       controller 的名称

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数

view_controller_chains
----------------------

.. code-block:: console

    $ ros2 control view_controller_chains -h
    usage: ros2 control view_controller_chains [-h] [--spin-time SPIN_TIME] [-s] [--save] [-c CONTROLLER_MANAGER] [--include-hidden-nodes] [--ros-args ...]

    生成已加载链式 controller 的示意图

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      --save                将示意图保存为当前目录下的 controller_diagram.pdf，而不是在查看器中打开
      -c CONTROLLER_MANAGER, --controller-manager CONTROLLER_MANAGER
                            controller manager ROS 节点的名称（默认: controller_manager）
      --include-hidden-nodes
                            同时考虑隐藏节点
      --ros-args ...        向可执行文件传递任意参数

示意图会在默认 PDF 查看器中打开。若没有可用查看器（例如通过 SSH 工作时），则会改为保存到当前目录下的 ``controller_diagram.pdf``；也可使用 ``--save`` 显式指定。无论哪种情况，都会打印所生成文件的路径。

controller 之间交换的 interface 会以前置导出 controller 的名称显示：标记为 ``(exp ref)`` 的 interface 是 chainable controller 导出、供前一 controller 写入的 reference interface；标记为 ``(exp state)`` 的则是其导出、供其他 controller 读取的 state interface。


view_hardware_status
--------------------

.. code-block:: console

    $ ros2 control view_hardware_status -h
    usage: ros2 control view_hardware_status [-h] [--spin-time SPIN_TIME] [-s] [-i HARDWARE_ID] [-d DEVICE_ID]

    回显 hardware 状态消息，并支持过滤

    选项:
      -h, --help            显示此帮助信息并退出
      --spin-time SPIN_TIME
                            等待发现的自旋时间（秒）（仅在不使用已在运行的 daemon 时适用）
      -s, --use-sim-time    启用 ROS 仿真时间
      -i HARDWARE_ID, --hardware-id HARDWARE_ID
                            按指定的 hardware component ID 过滤。
      -d DEVICE_ID, --device-id DEVICE_ID
                            按 hardware component 内指定的 device ID 过滤。