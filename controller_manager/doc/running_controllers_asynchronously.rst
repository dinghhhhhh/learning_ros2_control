:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/controller_manager/doc/running_controllers_asynchronously.rst

.. _running_controllers_asynchronously:

异步运行 Controller
====================================

``ros2_control`` 框架允许 controller 异步运行。
当某个 controller 的 ``update()`` 方法包含阻塞调用，或所需执行时间会影响 controller manager 的控制循环周期、
进而影响控制循环周期性并导致 overrun 时，这一点尤为有用。

例如，若 controller manager 的 ``update_rate`` 为 100Hz，则所有 controller 的 ``update()`` 调用与 hardware component 的 ``read()``、
``write()`` 调用的执行时间之和必须低于 10ms。若某个 controller 需要 15ms 的执行时间，则无法在不降低整体系统 update 速率的情况下同步执行。
以异步方式运行此类 controller 可使其在专用线程上执行，而不会阻塞主控制循环。

.. note::
  异步 update 支持对各 controller 实现是透明的。
  任何既有 controller 只需将 ``is_async`` parameter 设为 ``true`` 即可异步运行。

Parameters
-----------

可在 controller 的配置中设置以下 parameter，以使其异步运行：

* ``is_async``：（可选）若设为 ``true``，controller 将异步运行其 ``update()`` 方法。默认为 ``false``。
* ``update_rate``：（可选）以 Hz 为单位的 controller ``update()`` 触发频率。
  若未设置或设为 ``0``，controller 与 controller manager 以相同频率运行。

可在 ``async_parameters`` namespace 下设置额外的线程 parameter：

* ``thread_priority``：（可选）运行 controller ``update()`` 的线程优先级。
  优先级为 0 到 99 之间的整数值。若未设置，线程继承与 controller manager 线程相同的优先级。
* ``cpu_affinity``：（可选）将异步线程绑定到的 CPU 核心 ID 列表。
  默认为空列表，表示线程可在任意 CPU 核心上运行。

.. note::
  thread priority 仅在 controller 异步运行时使用。
  异步 controller 线程使用 FIFO 调度策略。

示例
---------

以下示例展示如何在 ROS 2 YAML parameter 文件中配置异步 controller。controller manager 以 100Hz 运行，异步 controller 配置为 20Hz：

.. code-block:: yaml

    controller_manager:
      ros__parameters:
        update_rate: 100  # Hz

    example_async_controller:
      ros__parameters:
        type: example_controller/ExampleController
        is_async: true
        update_rate: 20  # Hz

结果是 controller 以 20Hz 触发，而 controller manager 仍以 100Hz 运行。

可使用 ``async_parameters`` namespace 配置额外的线程 parameter：

.. code-block:: yaml

    example_async_controller:
      ros__parameters:
        type: example_controller/ExampleController
        is_async: true
        update_rate: 20  # Hz
        async_parameters:
          thread_priority: 50
          cpu_affinity: [2, 4]

所有 parameter 的说明见 ``ros2_controllers`` 文档中的 `Common Controller Parameters
<https://control.ros.org/master/doc/ros2_controllers/doc/controllers_index.html#common-controller-parameters>`_
一节。

调度行为
--------------------

从设计角度看，controller manager 充当调度器，在控制循环中触发异步 controller 的 update。

``ControllerInterfaceBase`` 调用 ``AsyncFunctionHandler`` 处理 controller 实际的 ``update()`` 回调。
``AsyncFunctionHandler`` 属于 `realtime_tools
<https://control.ros.org/rolling/doc/realtime_tools/doc/index.html>`__ 包。resource
manager 也使用同一机制支持异步 hardware component 的 ``read()`` 与 ``write()`` 操作（参见 :ref:`asynchronous_components`）。
当 controller 配置为异步运行时，controller interface 在 controller 的配置阶段创建 async handler，并将其绑定到 controller 的 ``update()`` 方法。
async handler 线程以与 controller manager 相同的线程优先级创建，或由 ``thread_priority`` parameter 指定的优先级创建。

当 controller manager 触发时，async handler 会检查上一次触发是否已完成，然后调用 ``update()`` 方法。
若在上一次 ``update()`` 仍在运行时下一次触发到达，将复用上一次的结果，controller manager 会打印警告：

.. code-block:: console

   [ros2_control_node-1] [WARN] [1741626670.311533972] [example_async_controller]: The controller missed xx update cycles out of yy total triggers.

若该警告频繁出现，应降低 controller 的 ``update_rate``，因为计算耗时已超过所配置周期所能容纳的范围。

若异步 controller 的 ``update()`` 方法抛出未处理异常，controller manager 的处理方式与同步 controller 相同——停用该 controller，并打印类似以下的错误消息：

.. code-block:: console

  [ros2_control_node-1] [ERROR] [1741629098.352771957] [AsyncFunctionHandler]: AsyncFunctionHandler: Exception caught in the async callback thread!
  ...
  [ros2_control_node-1] [ERROR] [1741629098.352874151] [controller_manager]: Caught exception of type : St13runtime_error while updating controller
  [ros2_control_node-1] [ERROR] [1741629098.352940701] [controller_manager]: Deactivating controllers : [example_async_controller] as their update resulted in an error!


另请参阅
---------

* :ref:`asynchronous_components` — 异步运行 hardware component
* :ref:`different_update_rates_userdoc` — 以不同 update 速率运行 hardware component