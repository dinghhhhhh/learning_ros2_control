:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/doc/release_notes.rst

发行说明：Kilted Kaiju 至 Lyrical Luth
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

本列表概述 Kilted Kaiju（上一版）与 Lyrical Luth（当前版）之间的重要变更。

controller_interface
********************
* 新的 ``MagneticFieldSensor`` semantic component 提供读取磁力计数据的 interface。（`#2627 <https://github.com/ros-controls/ros2_control/pull/2627>`__）
* 若链中某个 controller 在 update 周期失败，controller_manager 现将停用整条 controller 链。（`#2681 <https://github.com/ros-controls/ros2_control/pull/2681>`__）
* 当 controller 频率无法在当前 controller manager update 频率下实现时，现会将 controller 的 update 频率近似为更接近的可实现频率。（`#2828 <https://github.com/ros-controls/ros2_control/pull/2828>`__）
* lifecycle ID 在 controller 内部缓存，以避免在 realtime 控制循环中调用 get_lifecycle_state()。（`#2884 <https://github.com/ros-controls/ros2_control/pull/2884>`__）
* 新增 2 种 interface_configuration_types：``INDIVIDUAL_BEST_EFFORT`` 与 ``REGEX``，支持更灵活的 controller interface 配置。（`#2902 <https://github.com/ros-controls/ros2_control/pull/2902>`__）
* 新增 ``on_export_state_interfaces_list`` 与 ``on_export_reference_interfaces_list`` 方法，用于为 chainable controller 导出 interface 指针。（`#2988 <https://github.com/ros-controls/ros2_control/pull/2988>`__）

controller_manager
******************
* ``bcolors`` 类现尊重 ``RCUTILS_COLORIZED_OUTPUT`` 环境变量，在非 TTY 与 CI 环境中自动禁用颜色。
* ``switch_controller`` 的默认 strictness 改为 ``strict``。（`#2742 <https://github.com/ros-controls/ros2_control/pull/2742>`__）
* 为 controller manager 新增 parameter ``handle_exceptions``，用于控制 update 期间 controller 抛出的异常是在内部捕获处理还是向上传播。（`#2807 <https://github.com/ros-controls/ros2_control/pull/2807>`__）
* ``spawner`` 现支持在使用 ``--controller`` 选项解析多个 controller 参数时为每个 controller 单独指定参数。（`#2895 <https://github.com/ros-controls/ros2_control/pull/2895>`__）
* 为 controller manager 新增 ``cleanup_controller`` service，允许外部客户端清理 controller。（`#2414 <https://github.com/ros-controls/ros2_control/pull/2414>`__）
* 不再通过 NodeOptions 将 controller manager 的 ros 参数转发给 controller。（`#3016 <https://github.com/ros-controls/ros2_control/pull/3016>`__）
* ``spawner`` 现将解析到的所有 parameter 文件转发给被 spawn 的 controller，支持 ``allow_substs`` 方式。（`#3136 <https://github.com/ros-controls/ros2_control/pull/3136>`__）
* ``configure_controller`` 不再在配置前隐式清理 ``inactive`` controller；现严格要求 ``unconfigured`` 状态，否则返回错误。（`#3196 <https://github.com/ros-controls/ros2_control/pull/3196>`__）
* ``spawner`` 会跳过已配置的 controller。使用新的 ``--reconfigure`` 参数可再次 cleanup 并 configure。（`#3196 <https://github.com/ros-controls/ros2_control/pull/3196>`__）
* ``spawner`` 与 ``ros2 control load_controller`` 现接受 controller namespace 分散在嵌套键中的 parameter 文件（例如 ``/ns: {controller: {ros__parameters: ...}}``），无论是否带前导斜杠，并与 ``rclcpp`` 相同的方式匹配条目（包括名称中任意位置的 ``/*`` 与 ``/**`` 通配符）及其优先级。（`#3719 <https://github.com/ros-controls/ros2_control/pull/3719>`__）

hardware_interface
******************
* lifecycle ID 在 controller 内部缓存，以避免在 realtime 控制循环中调用 get_lifecycle_state()。（`#2884 <https://github.com/ros-controls/ros2_control/pull/2884>`__）
* Handle 除 double 与 bool 外，现还支持 ``float32``、``uint8``、``int8``、``uint16``、``int16``、``uint32``、``int32`` 数据类型。（`#2879 <https://github.com/ros-controls/ros2_control/pull/2879>`__）
* ``CommandInterface::is_limited()`` 现按值返回 ``bool`` 而非 ``const bool &``，因为 limited 标志现为 atomic，以避免与 introspection 采样器产生 data race。（`#3618 <https://github.com/ros-controls/ros2_control/issues/3618>`__）

ros2controlcli
**************
* 新增调用 controller cleanup 的 CLI 支持。（`#2414 <https://github.com/ros-controls/ros2_control/pull/2414>`__）
* 为 ``unload_controller`` 动词添加 ``--all-inactive`` 选项。（`#3466 <https://github.com/ros-controls/ros2_control/pull/3466>`__）

transmission_interface
**********************
* ``simple_transmission`` 与 ``differential_transmission`` 现也支持 ``force`` interface（`#2588 <https://github.com/ros-controls/ros2_control/pull/2588>`_）。