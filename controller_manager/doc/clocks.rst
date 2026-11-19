:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/controller_manager/doc/clocks.rst

.. _clocks:

Controller Manager 使用的不同 clock
===========================================

在非仿真的配置下，controller manager 在内部使用以下两种不同的 clock：

- ``RCL_ROS_TIME``：该 clock 主要用于非 realtime 循环。
- ``RCL_STEADY_TIME``：该 clock 主要用于 realtime 循环中的 ``read``、``update`` 和 ``write`` 循环。但当 controller manager 在仿真环境中使用时，``read``、``update`` 和 ``write`` 循环的触发使用 ``RCL_ROS_TIME`` clock。

hardware component 的 ``read`` 与 ``write`` 方法中的 ``time`` 参数类型为 ``RCL_STEADY_TIME``，因为大多数 hardware 期望时间为单调递增且不受系统时间变更影响。而 controller 的 ``update`` 方法中的 ``time`` 参数类型为 ``RCL_ROS_TIME``，因为 controller 会与其他 node 或 topic 交互以接收 command 或发布 state。controller 可利用该 ``time`` 参数校验收到的 command，或在正确的时间戳发布 state。
``read``、``update`` 和 ``write`` 方法中的 ``period`` 参数由 ``RCL_STEADY_TIME`` 类型的触发 clock 计算，因此始终单调递增。

使用不同 clock 的原因，是避免系统时间变更对 realtime 循环造成影响。``ros2_control_node`` 现在也会检测由系统时间变更以及 controller 与 hardware component 执行时间过长所导致的 overrun。若因系统时间变更或执行时间过长而导致 controller 或 hardware component 错过 update 周期，controller manager 将打印警告消息。