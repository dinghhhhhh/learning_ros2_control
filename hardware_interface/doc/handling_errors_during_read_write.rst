:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/doc/handling_errors_during_read_write.rst

Handling of Errors During read() and write() Calls
--------------------------------------------------
如果 hardware_interface 类的 ``read()`` 或 ``write()`` 方法返回 ``hardware_interface::return_type::ERROR``，将调用 ``on_error(previous_state)`` 方法以处理发生的任何错误。

错误处理遵循 `node lifecycle <https://design.ros2.org/articles/node_lifecycle.html>`_。
若返回 ``CallbackReturn::SUCCESS`` 且成功，则硬件会再次进入 ``UNCONFIGURED`` 状态；若发生任何 ``ERROR`` 或 ``FAILURE``，硬件将进入 ``FINALIZED`` 状态且无法恢复。
目前唯一的办法是重新加载整个 plugin，但 Controller Manager 中尚没有提供对应的服务。
