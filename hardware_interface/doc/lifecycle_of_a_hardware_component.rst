:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/doc/lifecycle_of_a_hardware_component.rst

Lifecycle of a Hardware Component
---------------------------------
方法的返回值类型为
``rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn``，含义如下：

* ``CallbackReturn::SUCCESS`` 方法执行成功。
* ``CallbackReturn::FAILURE`` 方法执行失败，lifecycle 状态转换不成功。
* ``CallbackReturn::ERROR`` 发生了严重错误，应在 ``on_error`` 方法中处理。

硬件在各方法执行后会转换到如下状态：

* **UNCONFIGURED** (``on_init``, ``on_cleanup``)：

  硬件仅完成初始化，通信尚未启动，且没有 interfaces 被导入到 ``ResourceManager``。

* **INACTIVE** (``on_configure``, ``on_deactivate``)：

  已与硬件建立通信，且 hardware component 已配置完成。
  可以读取 states，但 command interfaces（仅 System 与 Actuator）尚不可用。

  目前，是否继续使用从 ``CommandInterfaces`` 收到的 command，还是完全跳过，由 hardware component 的实现自行决定。

  .. note::

    我们计划实现安全关键的 interfaces，参见 roadmap 中的 `PR <https://github.com/ros-controls/roadmap/pull/51/files>`__。但目前所有 command interfaces 都可用且会被写入，参见描述该情况的 `issue <https://github.com/ros-controls/ros2_control/issues/931>`__。

* **FINALIZED** (``on_shutdown``)：

  Hardware interface 已准备好卸载/销毁。
  已分配的内存会被清理。

* **ACTIVE** (``on_activate``)：

  可以读取 states。

  仅对 System 与 Actuator：

    硬件的功率回路已激活，硬件可以运动，例如刹车已松开。
    Command interfaces 可用，命令应被发送到硬件。
