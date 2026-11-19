:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/doc/migration.rst

迁移指南：Kilted Kaiju 至 Lyrical Luth
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

本列表概述 Kilted Kaiju（上一版）与 Lyrical Luth（当前版）之间的重要变更，用户代码可能需要据此修改。


controller_interface
********************

ChainableControllerInterface
----------------------------

* 已移除 ``on_export_state_interfaces()`` 方法，由 ``on_export_state_interfaces_list()`` 替代（`#2988 <https://github.com/ros-controls/ros2_control/pull/2988>`_, `#3610 <https://github.com/ros-controls/ros2_control/pull/3610>`__）。新方法返回 shared pointer，而非按值返回对象：

  .. code-block:: cpp

     // Old (removed)
     std::vector<hardware_interface::StateInterface> on_export_state_interfaces()

     // New
     std::vector<hardware_interface::StateInterface::SharedPtr> on_export_state_interfaces_list()

  迁移示例：

  .. code-block:: cpp

     // Old implementation
     std::vector<hardware_interface::StateInterface>
     MyController::on_export_state_interfaces()
     {
       std::vector<hardware_interface::StateInterface> state_interfaces;
       state_interfaces.emplace_back(
         std::string(get_node()->get_name()) + "/my_state", "position", &my_state_value_);
       return state_interfaces;
     }

     // New implementation
     std::vector<hardware_interface::StateInterface::SharedPtr>
     MyController::on_export_state_interfaces_list()
     {
       my_state_itf_ = std::make_shared<hardware_interface::StateInterface>(
         std::string(get_node()->get_name()) + "/my_state", "position");
       std::ignore = my_state_itf_->set_value(std::numeric_limits<double>::quiet_NaN());
       return {my_state_itf_};
     }

* 已移除 ``on_export_reference_interfaces()`` 方法，由 ``on_export_reference_interfaces_list()`` 替代（`#2988 <https://github.com/ros-controls/ros2_control/pull/2988>`_, `#3610 <https://github.com/ros-controls/ros2_control/pull/3610>`__）。``on_export_reference_interfaces_list()`` 为纯虚函数，因此每个 chainable controller 都必须实现，即使不导出 reference interface。新方法返回 shared pointer，而非按值返回对象：

  .. code-block:: cpp

     // Old (removed)
     std::vector<hardware_interface::CommandInterface> on_export_reference_interfaces()

     // New
     std::vector<hardware_interface::CommandInterface::SharedPtr> on_export_reference_interfaces_list()

  迁移示例：

  .. code-block:: cpp

     // Old implementation
     std::vector<hardware_interface::CommandInterface>
     MyController::on_export_reference_interfaces()
     {
       reference_interfaces_.resize(1, std::numeric_limits<double>::quiet_NaN());
       std::vector<hardware_interface::CommandInterface> reference_interfaces;
       reference_interfaces.emplace_back(
         std::string(get_node()->get_name()) + "/my_ref", "velocity", &reference_interfaces_[0]);
       return reference_interfaces;
     }

     // New implementation
     std::vector<hardware_interface::CommandInterface::SharedPtr>
     MyController::on_export_reference_interfaces_list()
     {
       my_ref_itf_ = std::make_shared<hardware_interface::CommandInterface>(
         std::string(get_node()->get_name()) + "/my_ref", "velocity");
       std::ignore = my_ref_itf_->set_value(std::numeric_limits<double>::quiet_NaN());
       return {my_ref_itf_};
     }

* 导出的 state interface 现由 ``ChainableControllerInterface::export_state_interfaces()`` 以 ``ConstSharedPtr`` 返回，以确保消费者对之为只读（`#1767 <https://github.com/ros-controls/ros2_control/pull/1767>`_）。

* 已移除内部存储变量 ``reference_interfaces_`` 与 ``state_interfaces_values_``（`#2988 <https://github.com/ros-controls/ros2_control/pull/2988>`_, `#3610 <https://github.com/ros-controls/ros2_control/pull/3610>`__）。取值现保存在导出的 interface 自身内。将在 ``on_export_*_interfaces_list()`` 中创建的 shared pointer 保留为成员，或使用有序导出 interface 容器（``ordered_exported_state_interfaces_`` 与 ``ordered_exported_reference_interfaces_``），并在 ``update_reference_from_subscribers()``、``update_and_write_commands()`` 与 ``on_set_chained_mode()`` 中通过 ``get_optional()`` 与 ``set_value()`` 访问取值：

  .. code-block:: cpp

     // Old implementation
     controller_interface::return_type MyController::update_and_write_commands(
       const rclcpp::Time &, const rclcpp::Duration &)
     {
       if (!std::isnan(reference_interfaces_[0]))
       {
         std::ignore = command_interfaces_[0].set_value(reference_interfaces_[0]);
       }
       my_state_value_ = state_interfaces_[0].get_optional().value_or(my_state_value_);
       return controller_interface::return_type::OK;
     }

     // New implementation
     // my_ref_itf_ 与 my_state_itf_ 在 on_export_*_interfaces_list() 中保存
     controller_interface::return_type MyController::update_and_write_commands(
       const rclcpp::Time &, const rclcpp::Duration &)
     {
       const double reference =
         my_ref_itf_->get_optional().value_or(std::numeric_limits<double>::quiet_NaN());
       if (!std::isnan(reference))
       {
         std::ignore = command_interfaces_[0].set_value(reference);
       }
       if (const auto state = state_interfaces_[0].get_optional(); state.has_value())
       {
         std::ignore = my_state_itf_->set_value(state.value());
       }
       return controller_interface::return_type::OK;
     }

  若无法获取 interface 锁，``get_optional()`` 返回 ``std::nullopt``，``set_value()`` 返回 ``false``，因此应处理这些情况，而非假定访问总会成功。

  导出多个 interface 的 controller 可根据固定名称列表构建（`#3350 <https://github.com/ros-controls/ros2_control/issues/3350>`__），并使返回的指针索引方式与原先的 ``reference_interfaces_`` 向量一致：

  .. code-block:: cpp

     static constexpr std::array<std::string_view, 3> reference_interface_names = {
       "angular_velocity.x", "angular_velocity.y", "angular_velocity.z"};

     std::vector<hardware_interface::CommandInterface::SharedPtr>
     MyController::on_export_reference_interfaces_list()
     {
       ref_itfs_.clear();
       for (const auto & name : reference_interface_names)
       {
         auto itf = std::make_shared<hardware_interface::CommandInterface>(
           get_node()->get_name(), std::string{name});
         std::ignore = itf->set_value(std::numeric_limits<double>::quiet_NaN());
         ref_itfs_.push_back(itf);
       }
       return ref_itfs_;
     }

  `ros2_controllers#2350 <https://github.com/ros-controls/ros2_controllers/pull/2350>`__ 迁移了官方 chainable controller，可作为参考。

  chainable controller 迁移检查清单：

  #. 为现有的 ``on_export_state_interfaces()`` 与 ``on_export_reference_interfaces()`` 重写添加 ``override``。若旧方法未声明 ``override``，仍可编译但不会被调用，其 interface 会悄然未导出；添加 ``override`` 可使编译器标记所有必须修改的方法。
  #. 将 ``on_export_state_interfaces()`` 重命名为 ``on_export_state_interfaces_list()``，将 ``on_export_reference_interfaces()`` 重命名为 ``on_export_reference_interfaces_list()``，返回类型改为 ``std::vector<...::SharedPtr>``。
  #. 将每个 ``emplace_back(prefix, name, &storage)`` 替换为 ``std::make_shared<...>(prefix, name)``，用 ``set_value()`` 设置初始值，并将指针存为成员。每个导出 interface 的前缀必须以 controller 名称（``get_node()->get_name()``）开头，否则导出会抛出 ``std::runtime_error``。
  #. 即使没有 reference interface，也必须实现 ``on_export_reference_interfaces_list()``；此种情况返回空向量。
  #. 移除 ``reference_interfaces_.resize(...)`` 与 ``state_interfaces_values_.resize(...)`` 调用，通常位于 ``on_configure()`` 或导出方法中。
  #. 将所有对 ``reference_interfaces_[i]`` 的读取替换为 ``ref_itfs_[i]->get_optional()``，写入替换为 ``ref_itfs_[i]->set_value(...)``。对曾经支撑导出 state interface 的成员变量同样处理。检查 ``update_reference_from_subscribers()``、``update_and_write_commands()``、``on_set_chained_mode()``、``on_activate()`` 与 ``on_deactivate()``，以及 controller 测试。
  #. 在 controller 源码中搜索 ``reference_interfaces_``、``state_interfaces_values_``、``on_export_state_interfaces()`` 与 ``on_export_reference_interfaces()``；不应再有匹配。同一包中的 Hardware Component 仍使用 ``on_export_state_interfaces()``，那里的匹配可忽略。

controller_manager
******************

* ``configure_controller`` 现执行单步 lifecycle 转换，且仅接受处于 ``unconfigured`` 状态的 controller（`#3196
  <https://github.com/ros-controls/ros2_control/pull/3196>`__）。此前 ``inactive`` controller 会被隐式 cleanup 再 configure。要重新配置 ``inactive`` controller，须先调用 ``cleanup_controller``，再调用 ``configure_controller``。

* controller manager 的 ros 参数不再通过 NodeOptions 转发给 controller。（`#3016 <https://github.com/ros-controls/ros2_control/pull/3016>`__）
  因此，在 controller manager 级别所做的 remapping 对 controller 不再可见。
  建议使用 spawner 的 ``--controller-ros-args`` 选项向 controller 传递 ros 参数。

  .. code-block:: python

    control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[robot_controllers],
        remappings=[("/diffbot_base_controller/cmd_vel", "/cmd_vel")],
        output="both",
    )

  改为

  .. code-block:: python

    control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[robot_controllers],
        output="both",
    )
    spawner_node = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "diffbot_base_controller",
            "--controller-ros-args",
            "--remap",
            "/diffbot_base_controller/cmd_vel:=/cmd_vel",
        ],
    )

hardware_interface
******************

* 所有 ``hardware_interface::*Interface`` 类中 ``on_init`` 方法的签名已变更（`#2323
  <https://github.com/ros-controls/ros2_control/pull/2323>`_,
  `#2589 <https://github.com/ros-controls/ros2_control/pull/2589>`__），由

  .. code-block:: cpp

     CallbackReturn on_init(const hardware_interface::HardwareInfo& info)

  变为

  .. code-block:: cpp

     CallbackReturn on_init(const HardwareComponentInterfaceParams& params)

  可通过 ``params.hardware_info`` 从 ``HardwareComponentInterfaceParams`` 对象访问 ``HardwareInfo`` 对象。``HardwareComponentInterfaceParams`` 的高级用法见 :ref:`writing_new_hardware_component`。

* 所有 ``hardware_interface::*Interface`` 类中 ``init()`` 方法的签名已变更（`#2344
  <https://github.com/ros-controls/ros2_control/pull/2344>`_,
  `#2589 <https://github.com/ros-controls/ros2_control/pull/2589>`__），由


  .. code-block:: cpp

     CallbackReturn init(const HardwareInfo & hardware_info, rclcpp::Logger logger, rclcpp::Clock::SharedPtr clock)

  变为

  .. code-block:: cpp

     CallbackReturn init(const hardware_interface::HardwareComponentParams & params)


* 所有 Hardware Component（如 ``Actuator``、``Sensor`` 等）的 ``initialize`` 方法已由传递 ``const HardwareInfo &`` 改为传递 ``const
  HardwareComponentParams &`` (`#2323 <https://github.com/ros-controls/ros2_control/pull/2323>`_,
  `#2589 <https://github.com/ros-controls/ros2_control/pull/2589>`__）。

* LoanedStateInterface 与 LoanedCommandInterface 的 ``get_value`` 现通过 ``get_optional`` 方法访问。取值以 ``std::optional<T>`` 返回。（`#2061 <https://github.com/ros-controls/ros2_control/pull/2061>`_）

  此变更是为了更好地处理因系统中其他线程并发访问而导致 interface 取值不可访问的情况。

* 标准 StateInterface 与 CommandInterface 的 ``double get_value()`` 现通过 ``get_optional`` 或 ``bool get_value(T & value, bool wait_for_lock)`` 访问。使用 ``get_optional`` 时取值为 ``std::optional<T>`` (`#2831 <https://github.com/ros-controls/ros2_control/pull/2831>`_)。

  同样，``set_value`` 已更新为 ``bool set_value(const T & value, bool wait_for_lock)``，返回值表示操作成功与否（`#2831 <https://github.com/ros-controls/ros2_control/pull/2831>`_）。

  可使用这些方法的返回值处理因并发访问导致 interface 取值不可访问的情况。可将 ``wait_for_lock`` parameter 设为 ``true`` 以阻塞直至获得锁，但这不是 realtime 安全的，在 realtime 上下文中应谨慎使用。

* 已移除 ``HardwareComponentInterface`` 的 ``export_state_interfaces()`` 与 ``export_command_interfaces()`` 方法（`#3610 <https://github.com/ros-controls/ros2_control/pull/3610>`__）。Hardware Component 不能再导出由自身成员变量支撑的 interface。``ros2_control`` 标签中定义的 interface 由框架创建并导出，其取值通过 ``set_state``/``get_state`` 与 ``set_command``/``get_command`` 访问：

  .. code-block:: cpp

     // Old implementation
     std::vector<hardware_interface::StateInterface> MyHardware::export_state_interfaces()
     {
       std::vector<hardware_interface::StateInterface> state_interfaces;
       state_interfaces.emplace_back(
         info_.joints[0].name, hardware_interface::HW_IF_POSITION, &hw_position_);
       return state_interfaces;
     }

     hardware_interface::return_type MyHardware::read(const rclcpp::Time &, const rclcpp::Duration &)
     {
       hw_position_ = read_position_from_hardware();
       return hardware_interface::return_type::OK;
     }

     // New implementation, no export method needed
     hardware_interface::return_type MyHardware::read(const rclcpp::Time &, const rclcpp::Duration &)
     {
       set_state(info_.joints[0].name + "/" + hardware_interface::HW_IF_POSITION,
                 read_position_from_hardware());
       return hardware_interface::return_type::OK;
     }

  未在 ``ros2_control`` 标签中列出的 interface 通过重写 ``export_unlisted_state_interface_descriptions()`` 或 ``export_unlisted_command_interface_descriptions()`` 添加。仅当需要完全控制导出 interface 时，才重写 ``on_export_state_interfaces()`` 或 ``on_export_command_interfaces()``。详见 :ref:`writing_new_hardware_component`。

  对旧存储向量的循环可映射为对框架 interface 映射的循环（``joint_state_interfaces_``、``joint_command_interfaces_``，以及 ``sensor_``、``gpio_``、``unlisted_`` 的对应项），键为完全限定 interface 名称，例如 ``prefix/joint_1/velocity``。以下示例假定映射中所有 interface 均为 ``double`` 类型；对其他数据类型的 interface，``set_state`` 会抛出异常：

  .. code-block:: cpp

     // Old implementation
     for (size_t i = 0; i < hw_states_.size(); ++i)
     {
       hw_states_[i] = 0.0;
     }
     hw_commands_[x] = hw_states_[y];

     // New implementation
     for (const auto & [name, descr] : joint_state_interfaces_)
     {
       set_state(name, 0.0);
     }
     set_command(name_of_command_interface_x, get_state(name_of_state_interface_y));

  Hardware Component 迁移检查清单：

  #. 为现有的 ``export_state_interfaces()`` 与 ``export_command_interfaces()`` 重写添加 ``override``。若旧方法未声明 ``override``，仍可编译但不会被调用，其 interface 会悄然未导出；添加 ``override`` 可使编译器标记所有必须修改的方法。
  #. 删除 ``export_state_interfaces()`` 与 ``export_command_interfaces()`` 重写。
  #. 确保它们导出的每个 interface 均列在 URDF 的 ``ros2_control`` 标签中；其余移至 ``export_unlisted_state_interface_descriptions()`` 或 ``export_unlisted_command_interface_descriptions()``。
  #. 在 ``read()`` 中，将对曾支撑 state interface 的成员变量赋值（例如 ``hw_positions_[i] = ...``）替换为 ``set_state("<joint>/<interface>", value)``。
  #. 在 ``write()`` 中，将对曾支撑 command interface 的成员变量的读取替换为 ``get_command<double>("<joint>/<interface>")``。在 ``on_activate()``、``on_deactivate()`` 与 ``perform_command_mode_switch()`` 中同样处理。
  #. 删除现已未使用的存储向量（``hw_states_``、``hw_commands_`` 等）及其在 ``on_init()`` 中的 ``resize()`` 调用。
  #. 在包内搜索 ``export_state_interfaces`` 与 ``export_command_interfaces``，以及旧存储成员的地址（例如 ``&hw_``）；不应再有匹配。

  自 Jazzy 起即可进行此迁移，参见 `Jazzy 迁移指南 <https://control.ros.org/jazzy/doc/ros2_control/doc/migration.html#migration-of-command-stateinterfaces>`__。

* 已移除 ``StateInterface`` 与 ``CommandInterface`` 的 ``Handle(prefix_name, interface_name, double * value_ptr)`` 构造函数（`#3610 <https://github.com/ros-controls/ros2_control/pull/3610>`__）。Handle 现始终拥有其取值。可从 ``InterfaceDescription`` 构造，或使用 ``Handle(prefix_name, interface_name, data_type, initial_value)``，其中 ``data_type`` 与 ``initial_value`` 为字符串，默认 ``"double"`` 与 ``""`` (NaN)。对 double interface，``Handle(prefix_name, interface_name)`` 即可。

* 已移除 ``Handle::operator bool()``（`#3610 <https://github.com/ros-controls/ros2_control/pull/3610>`__）。请改用 ``is_valid()``。

* 若 ``T`` 与 handle 的数据类型不匹配（包括 ``double``），handle 上的 ``set_value<T>()`` 现会抛出 ``std::runtime_error``（`#3610 <https://github.com/ros-controls/ros2_control/pull/3610>`__）。该错误在运行时抛出，而非编译时：在 double interface 上 ``set_value(0)`` 会抛出，因为 ``0`` 为 ``int``；应使用 ``set_value(0.0)``。``set_state`` 与 Hardware Component 的 ``set_command`` 同样适用。