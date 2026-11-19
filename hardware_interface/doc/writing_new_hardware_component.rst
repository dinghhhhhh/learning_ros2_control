:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/doc/writing_new_hardware_component.rst

.. _writing_new_hardware_component:

编写 Hardware Component
=========================

在 ros2_control 中，hardware system component 是库，由 Controller Manager 通过 `pluginlib <https://ros.org/wiki/pluginlib>`_ interface 动态加载。
下文是按步骤创建新 hardware interface 的源文件、基础测试与编译规则的指南。

#. **准备软件包**

   若 hardware interface 的软件包尚不存在，请先创建。
   该包应使用 ``ament_cmake`` 作为 build type。
   最简单的方式是在网上搜索最新手册。
   ``ros2 pkg create`` 命令可帮助完成该过程。
   使用 ``--help`` 查看用法；也可选择一并生成库源文件与编译规则，便于后续步骤。

#. **准备源文件**

   创建软件包后，其中至少应包含 ``CMakeLists.txt`` 与 ``package.xml``。
   若尚不存在，还应创建 ``include/<PACKAGE_NAME>/`` 与 ``src`` 目录。
   在 ``include/<PACKAGE_NAME>/`` 下添加 ``<robot_hardware_interface_name>.hpp``，在 ``src`` 下添加 ``<robot_hardware_interface_name>.cpp``。

#. **在头文件 (.hpp) 中添加声明**

   1. 注意使用 header guards。ROS 2 风格使用 ``#ifndef`` 与 ``#define`` 预处理指令。（更多信息可借助搜索引擎 :） ）。

   2. 包含 ``"hardware_interface/$interface_type$_interface.hpp"``。
      ``$interface_type$`` 可为 ``Actuator``、``Sensor`` 或 ``System``，取决于所用 hardware 类型。各类型的更多细节见 :ref:`Hardware Components description <overview_hardware_components>`。

   3. 为 hardware_interface 定义唯一命名空间，通常与包名一致且采用 ``snake_case``。

   4. 定义 hardware_interface 类，继承 ``$InterfaceType$Interface``，例如：
      .. code:: c++
      class HardwareInterfaceName : public hardware_interface::$InterfaceType$Interface

   5. 添加无参构造函数，并实现 ``LifecycleNodeInterface`` 的下列 public 方法：``on_configure``、``on_cleanup``、``on_shutdown``、``on_activate``、``on_deactivate``、``on_error``；同时重写 ``$InterfaceType$Interface`` 的定义：``on_init``、``prepare_command_mode_switch`` (可选)、``perform_command_mode_switch`` (可选)、``read``、``write``。

     关于 hardware Lifecycle 的进一步说明见该 `pull request <https://github.com/ros-controls/ros2_control/pull/559/files#diff-2bd171d85b028c1b15b03b27d4e6dcbb87e52f705042bf111840e7a28ab268fc>`_；方法精确定义见 ``"hardware_interface/$interface_type$_interface.hpp"`` 头文件，或 *Actuator*、*Sensor*、*System* 的 `doxygen documentation <https://control.ros.org/{REPOS_FILE_BRANCH}/doc/api/namespacehardware__interface.html>`_。

#. **在源文件 (.cpp) 中添加定义**

   #. 包含 hardware interface 的头文件，并定义命名空间以简化后续开发。

   #. 实现 ``on_init`` 方法。在此初始化所有成员变量并处理 ``info`` 参数中的参数。
      第一行通常调用父类的 ``on_init`` 以处理标准值（如 name），使用：``hardware_interface::(Actuator|Sensor|System)Interface::on_init(info)``。
      若所有必需参数均已设置且有效、一切正常，则返回 ``CallbackReturn::SUCCESS``，否则 ``return CallbackReturn::ERROR``。

      #. **（可选）添加 Publisher、Service 等**

         对 hardware component 的常见需求是在不干扰实时控制循环的情况下发布状态或诊断信息。

         这样可在 hardware interface 中加入任意标准 ROS 2 组件（publisher、subscriber、service、timer），而不损害实时性能。主要有三种方式。

         **方法 1：使用 Framework-Managed Publisher（推荐，且最适合 HardwareStatus 消息）**

            参见 :ref:`Framework Managed Publisher <framework_managed_publisher>`

         **方法 2：使用 Framework-Managed Node（推荐，且最适合自定义消息）**

            框架在内部为每个 hardware component 创建专用 ROS 2 node。hardware plugin 可获取该 node 的句柄并使用。

            #. **访问并使用默认 Node**：调用 ``get_node()`` 可获得 ``shared_ptr`` 形式的 node，并像任意 ``rclcpp::Node::SharedPtr`` 一样创建 publisher、timer 等。

               .. code-block:: cpp

                  // 继续在 on_configure() 内
                  if (get_node())
                  {
                     my_publisher_ = get_node()->create_publisher<std_msgs::msg::String>("~/status", 10);

                     using namespace std::chrono_literals;
                     my_timer_ = get_node()->create_wall_timer(1s, [this]() {
                        std_msgs::msg::String msg;
                        msg.data = "Hardware status update!";
                        my_publisher_->publish(msg);
                     });
                  }

         **方法 3：使用 `HardwareComponentInterfaceParams` 中的 Executor**

            若需对 node 创建有更直接控制的高级场景，可将 ``on_init`` 配置为接收 ``HardwareComponentInterfaceParams`` 结构体，其中包含指向 ``ControllerManager`` 的 executor 的 ``weak_ptr``。

            #. **更新 ``on_init`` 签名**：hardware interface 须重写接受 ``HardwareComponentInterfaceParams`` 的 ``on_init`` 版本。

               .. code-block:: cpp

                  // 在 <robot_hardware_interface_name>.hpp 中
                  hardware_interface::CallbackReturn on_init(
                  const hardware_interface::HardwareComponentInterfaceParams & params) override;

            #. **锁定并使用 Executor**：在 ``on_init`` 内须安全地“锁定” ``weak_ptr`` 以获得可用的 ``shared_ptr``，然后可自行创建 node 并加入 executor。

               .. code-block:: cpp

                  // 在 <robot_hardware_interface_name>.cpp 的 on_init(params) 内
                  if (auto locked_executor = params.executor.lock())
                  {
                     my_custom_node_ = std::make_shared<rclcpp::Node>("my_custom_node");
                     locked_executor->add_node(my_custom_node_->get_node_base_interface());
                     // ... 在 my_custom_node_ 上创建 publisher/timer ...
                  }

            使用 framework-managed node 发布诊断消息的完整可运行示例见 :ref:`Example 17 <ros2_control_demos_example_17_userdoc>` 中的 demo。

   #. 编写 ``on_configure`` 方法，通常在此建立与 hardware 的通信并完成激活前的各项设置。

   #. 实现 ``on_cleanup`` 方法，其作用与 ``on_configure`` 相反。
   #. ``Command-/StateInterface`` 现由框架根据 ``ros2_control`` XML 标签中定义的 interface，通过 ``on_export_command_interfaces()`` 或 ``on_export_state_interfaces()`` 自动创建并导出；XML 被解析并据此创建 ``InterfaceDescription`` (参见 `hardware_info.hpp <https://github.com/ros-controls/ros2_control/tree/{REPOS_FILE_BRANCH}/hardware_interface/include/hardware_interface/hardware_info.hpp>`__）。

      * 访问自动创建的 ``Command-/StateInterface`` 时，我们提供 ``std::unordered_map<std::string, InterfaceDescription>``：string 为 interface 的全限定名，``InterfaceDescription`` 为 interface 配置。``std::unordered_map<>`` 分为 ``type_state_interfaces_`` 与 ``type_command_interfaces_``，其中 type 可为：``joint``、``sensor``、``gpio``、``unlisted``。例如所有 joint 的 ``CommandInterface`` 可在 ``joint_command_interfaces_`` map 中找到。``unlisted`` 包含未在 ``ros2_control`` XML 标签中列出、但通过重写 ``export_unlisted_command_interface_descriptions()`` 或 ``export_unlisted_state_interface_descriptions()`` 并创建自定义 ``Command-/StateInterface`` 而产生的 interface。
      * ``Sensor`` 类型的 hardware interface 不导出 command interface；``SensorInterface`` 已重写 ``on_export_command_interfaces()`` 并返回空 vector。
      * 提醒：完整 interface 名称的结构为 ``<joint_name>/<interface_type>``。

   #. （可选）若需要未列入 ``ros2_control`` XML 标签的 unlisted ``Command-/StateInterface``，可按下列步骤操作：

      #. 重写 ``virtual std::vector<hardware_interface::InterfaceDescription> export_unlisted_command_interface_descriptions()`` 或 ``virtual std::vector<hardware_interface::InterfaceDescription> export_unlisted_state_interface_descriptions()``
      #. 在重写的 ``export_unlisted_command_interface_descriptions()`` 或 ``export_unlisted_state_interface_descriptions()`` 中为每个要创建的 interface 构造 ``InterfaceDescription``，加入 vector 并返回：

         .. code-block:: c++

            std::vector<hardware_interface::InterfaceDescription> my_unlisted_interfaces;

            InterfaceInfo unlisted_interface;
            unlisted_interface.name = "some_unlisted_interface";
            unlisted_interface.min = "-5.0";
            unlisted_interface.data_type = "double";
            my_unlisted_interfaces.push_back(InterfaceDescription(info_.name, unlisted_interface));

            return my_unlisted_interfaces;

      #. unlisted interface 随后存入 ``unlisted_command_interfaces_`` 或 ``unlisted_state_interfaces_`` map，取决于在哪个函数中创建。
      #. 可像其他 interface 一样通过 ``get_state(name)``、``set_state(name, value)``、``get_command(name)`` 或 ``set_command(name, value)`` 访问，例如 ``get_state("some_unlisted_interface")``。

   #. （可选）若导出 ``Command-/StateInterface`` 的默认实现（``on_export_command_interfaces()`` 或 ``on_export_state_interfaces()``）不足，可重写它们。但应注意：

      * 若需 unlisted interface，须调用 ``export_unlisted_command_interface_descriptions()`` 或 ``export_unlisted_state_interface_descriptions()`` 并将其加入 ``unlisted_command_interfaces_`` 或 ``unlisted_state_interfaces_``。
      * 勿忘在内部保存创建的 ``Command-/StateInterface``：你仅返回 shared_ptr，ResourceManager 不会为 hardware 提供对这些 ``Command-/StateInterface`` 的访问，因此须自行保存。
      * 名称必须唯一！

   #.  （可选）对 *Actuator* 与 *System* 类型的 hardware interface，若 hardware 支持多种控制模式，可实现 ``prepare_command_mode_switch`` 与 ``perform_command_mode_switch``。

   #.  实现 ``on_activate``，在此启用 hardware“上电”。

   #.  实现 ``on_deactivate``，其作用与 ``on_activate`` 相反。

   #.  实现 ``on_shutdown``，在此优雅关闭 hardware。

   #.  实现 ``on_error``，在此处理各状态下出现的不同错误。

   #.  实现 ``read`` 方法：从 hardware 读取状态，并通过 ``set_state(name, value)`` 写入 state interface。

   #.  实现 ``write`` 方法：根据从 command interface 用 ``get_command(name)`` 读到的值向 hardware 下发命令。

   #. （可选）**Framework Managed Publisher**

      .. _framework_managed_publisher:

      实现 ``init_hardware_status_message`` 与 ``update_hardware_status_message``，通过 ``control_msgs/msg/HardwareStatus`` 消息发布框架支持的状态上报：

      *   **`init_hardware_status_message`**：该非实时方法在初始化时调用一次。须重写它以定义状态消息的**静态结构**，包括设置 ``hardware_id``、调整 ``hardware_device_states`` vector 大小，以及对每个设备调整其状态 vector（如 ``generic_hardware_status``、``canopen_states``）并填充 ``device_id``、interface ``name`` 等静态字段。在此处预分配消息结构对实时安全至关重要。

         .. code-block:: cpp

            // 在 <robot_hardware_interface_name>.hpp 中
            hardware_interface::CallbackReturn init_hardware_status_message(
            control_msgs::msg::HardwareStatus & msg_template) override;

            // 在 <robot_hardware_interface_name>.cpp 中
            hardware_interface::CallbackReturn MyHardware::init_hardware_status_message(
            control_msgs::msg::HardwareStatus & msg)
            {
               msg.hardware_id = get_hardware_info().name;
               msg.hardware_device_states.resize(get_hardware_info().joints.size());

               for (size_t i = 0; i < get_hardware_info().joints.size(); ++i)
               {
                  msg.hardware_device_states[i].device_id = get_hardware_info().joints[i].name;
                  // 本示例为每个 joint 使用一个 generic status
                  msg.hardware_device_states[i].generic_hardware_status.resize(1);
               }
               return hardware_interface::CallbackReturn::SUCCESS;
            }

      *   **`update_hardware_status_message`**：该实时安全方法由框架的 timer 回调调用。须重写它以**填入**预构建消息的动态值，通常将内部状态变量（在 `read()` 中更新）复制到消息字段。此方法必须快速且不得分配内存。

         .. code-block:: cpp

            // 在 <robot_hardware_interface_name>.hpp 中
            hardware_interface::return_type update_hardware_status_message(
            control_msgs::msg::HardwareStatus & msg) override;

            // 在 <robot_hardware_interface_name>.cpp 中
            hardware_interface::return_type MyHardware::update_hardware_status_message(
            control_msgs::msg::HardwareStatus & msg)
            {
               for (size_t i = 0; i < get_hardware_info().joints.size(); ++i)
               {
                  auto & generic_status = msg.hardware_device_states[i].generic_hardware_status;
                  // 示例：将内部状态映射到标准 status 字段
                  if (std::abs(hw_positions_[i]) > joint_limits_[i].max_position)
                  {
                     generic_status.health_status = control_msgs::msg::GenericState::HEALTH_ERROR;
                  }
                  else
                  {
                     generic_status.health_status = control_msgs::msg::GenericState::HEALTH_OK;
                  }
               }
               return hardware_interface::return_type::OK;
            }

      *   **在 URDF 中启用**：最后，在 URDF 的 ``<hardware>`` 标签中添加 ``status_publish_rate`` 参数以启用 publisher；设为 0.0 则禁用该功能。

         .. code-block:: xml

            <ros2_control name="MyHardware" type="system">
            <hardware>
               <plugin>my_package/MyHardware</plugin>
               <param name="status_publish_rate">20.0</param>
            </hardware>
            ...
            </ros2_control>

      使用 framework-managed node 发布诊断消息的完整可运行实现见 :ref:`Example 17 <ros2_control_demos_example_17_userdoc>` 中的 demo。

   #.  **重要**：在文件末尾、命名空间关闭之后，添加 ``PLUGINLIB_EXPORT_CLASS`` 宏。

      需要包含 ``"pluginlib/class_list_macros.hpp"`` 头文件。
      第一个参数应为确切的 hardware interface 类，例如 ``<my_hardware_interface_package>::<RobotHardwareInterfaceName>``；第二个参数为基类，即 ``hardware_interface::(Actuator|Sensor|System)Interface``。

#. **编写 pluginlib 导出定义**

   #. 在包中创建 ``<my_hardware_interface_package>.xml``，并添加库与 hardware interface 类的定义，使其对 pluginlib 可见。
      最简单的方式是参考 :ref:`hardware_interface mock_components <mock_components_userdoc>` 一节中 mock component 的定义。

   #. 通常 plugin 名称由包（命名空间）与类名组成，例如
      ``<my_hardware_interface_package>/<RobotHardwareInterfaceName>``。
      该名称决定 ResourceManager 查找时的 hardware interface 类型。
      另两个参数须与 ``<robot_hardware_interface_name>.cpp`` 文件底部宏中的定义一致。

#. **编写简单测试以确认 controller 可被找到并加载**

   #. 若尚不存在，在包中创建 ``test`` 文件夹，并添加 ``test_load_<robot_hardware_interface_name>.cpp``。

   #. 可复制 `test_generic_system.cpp <https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/test/mock_components/test_generic_system.cpp#L402-L407>`_ 包中定义的 ``load_generic_system_2dof`` 内容。

   #. 修改所复制测试的名称，并在最后一行指定 hardware interface 类型时，使用 ``<my_hardware_interface_package>.xml`` 中定义的名称，例如 ``<my_hardware_interface_package>/<RobotHardwareInterfaceName>``。

#. **在 ``CMakeLists.txt`` 中添加编译指令**

   #. 在 ``find_package(ament_cmake REQUIRED)`` 行之下添加更多依赖。
      至少包括：``hardware_interface``、``pluginlib``、``rclcpp``、``rclcpp_lifecycle``。

   #. 添加 shared library 的编译指令，以 ``<robot_hardware_interface_name>.cpp`` 为源文件。

   #. 为库添加 targeted include 目录，通常仅为 ``include``。

   #. 添加库所需的 ament 依赖，至少包含第 1 步所列项。

   #. 使用下列命令导出 pluginlib 描述文件：
      .. code:: cmake

         pluginlib_export_plugin_description_file(hardware_interface <my_hardware_interface_package>.xml)

   #. 为 target 与 include 目录添加 install 指令。

   #. 在 test 部分添加以下依赖：``ament_cmake_gmock``、``hardware_interface``。

   #. 使用 ``ament_add_gmock`` 为测试添加编译定义。
      细节可参考 `ros2_control <https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/CMakeLists.txt>`_ 包中 mock hardware 的做法。

   #. （可选）在 ``ament_package()`` 之前将 hardware interface 的库加入 ``ament_export_libraries``。

#. **在 ``package.xml`` 中添加依赖**

   #. 在 ``<depend>`` 标签中至少添加：``hardware_interface``、``pluginlib``、``rclcpp``、``rclcpp_lifecycle``。

   #. 在 ``<test_depend>`` 标签中至少添加：``ament_add_gmock``、``hardware_interface``。

#.  **编译并测试 hardware component**

   #. 此时可使用 ``colcon build <my_hardware_interface_package>`` 编译 hardware component。
      执行前请进入工作空间根目录。

   #. 若编译成功，source install 目录下的 ``setup.bash``，并执行 ``colcon test <my_hardware_interface_package>``，以确认新 controller 可通过 ``pluginlib`` 库被找到并由 Controller Manager 加载。


完成！祝你编写出色的 controller！

有用的外部参考
--------------

- `Templates and scripts for generating controllers shell <https://rtw.b-robotized.com/master/use-cases/ros2_control/setup_robot_hardware_interface.html>`_

  .. NOTE:: 该脚本目前仅推荐在 Foxy 与 Humble 上使用，与 Jazzy 及之后版本的 API 不兼容。