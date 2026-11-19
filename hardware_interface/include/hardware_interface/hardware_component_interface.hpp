// Copyright 2025 ros2_control Development Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef HARDWARE_INTERFACE__HARDWARE_COMPONENT_INTERFACE_HPP_
#define HARDWARE_INTERFACE__HARDWARE_COMPONENT_INTERFACE_HPP_

#include <fmt/format.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "control_msgs/msg/hardware_status.hpp"
#include "hardware_interface/component_parser.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/introspection.hpp"
#include "hardware_interface/types/hardware_component_interface_params.hpp"
#include "hardware_interface/types/hardware_component_params.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "hardware_interface/types/lifecycle_state_names.hpp"
#include "hardware_interface/types/trigger_type.hpp"
#include "lifecycle_msgs/msg/state.hpp"
#include "rclcpp/clock.hpp"
#include "rclcpp/duration.hpp"
#include "rclcpp/logger.hpp"
#include "rclcpp/logging.hpp"
#include "rclcpp/node.hpp"
#include "rclcpp/node_interfaces/node_clock_interface.hpp"
#include "rclcpp/node_options.hpp"
#include "rclcpp/time.hpp"
#include "rclcpp/version.h"
#include "rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp"
#include "rclcpp_lifecycle/state.hpp"
#include "realtime_tools/async_function_handler.hpp"
#include "realtime_tools/realtime_publisher.hpp"
#include "realtime_tools/realtime_thread_safe_box.hpp"

namespace hardware_interface
{

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

/**
 * @brief 所有 Hardware Component（Actuator、Sensor、System）的虚基类。
 *
 * 本类为所有 Hardware Component 提供通用结构与功能，包括 Lifecycle 管理、
 * interface 处理以及异步支持。Hardware 插件应继承其派生类之一：ActuatorInterface、
 * SensorInterface 或 SystemInterface。
 */
class HardwareComponentInterface : public rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface
{
public:
  HardwareComponentInterface();

  /// 显式删除 HardwareComponentInterface 拷贝构造函数。
  /**
   * Hardware interface 具有唯一所有权，不可拷贝，以避免
   * 对硬件的失败访问或并发访问。
   */
  HardwareComponentInterface(const HardwareComponentInterface & other) = delete;

  HardwareComponentInterface(HardwareComponentInterface && other) = delete;

  virtual ~HardwareComponentInterface();

  /// 根据从机器人 URDF 解析的数据以及 clock 与 logger interface 初始化 Hardware interface。
  /**
   * \param[in] params  类型为 HardwareComponentParams 的结构体，包含初始化该
   * Hardware Component 所需的全部参数，
   * 包括其 HardwareInfo、专用 logger、clock，以及
   * 指向 executor 的 weak_ptr。
   * \warning 传入的 executor 不得用于调用 `cancel()` 或使用阻塞型回调
   *（例如 `spin()`）。
   * \returns 若所需数据已提供且可解析，则返回 CallbackReturn::SUCCESS。
   * \returns 若发生任何错误或数据缺失，则返回 CallbackReturn::ERROR。
   */
  CallbackReturn init(const hardware_interface::HardwareComponentParams & params);

  /// 用户可覆盖的方法，用于配置 HardwareStatus 消息的结构。
  /**
   * 要启用状态发布，请覆盖本方法以预分配消息结构
   * 并填入设备 ID、interface 名称等静态信息。本方法在
   * 非实时 `init()` 阶段调用一次。若 `hardware_device_states` 向量
   * 保持为空，则将禁用发布。
   *
   * \param[out] msg_template 待配置的 HardwareStatus 消息引用。
   * \returns 配置成功返回 CallbackReturn::SUCCESS，失败返回 CallbackReturn::ERROR。
   */
  virtual CallbackReturn init_hardware_status_message(
    control_msgs::msg::HardwareStatus & msg_template);

  /// 用户可覆盖的方法，用实时数据填充 hardware 状态消息。
  /**
   * 框架在 `trigger_read()` 循环内调用本实时安全方法。
   * 覆盖本方法以在预分配的消息中填充 `value` 字段，写入
   * 在 `read()` 方法中更新的最新 hardware State。
   *
   * \param[in,out] msg 预分配的消息，将填入最新值。
   * \returns 成功返回 return_type::OK，失败返回 return_type::ERROR。
   */
  virtual return_type update_hardware_status_message(control_msgs::msg::HardwareStatus & msg);

  /// 根据从机器人 URDF 解析的数据初始化 Hardware interface。
  /**
   * \param[in] params  类型为 hardware_interface::HardwareComponentInterfaceParams
   * 的结构体，包含初始化该 Hardware Component 所需的全部参数，
   * 尤其是其 HardwareInfo，以及指向 executor 的 weak_ptr。
   * \warning 传入的 executor 不得用于调用 `cancel()` 或使用阻塞型回调
   *（例如 `spin()`）。
   * \returns 若所需数据已提供且可解析，则返回 CallbackReturn::SUCCESS。
   * \returns 若发生任何错误或数据缺失，则返回 CallbackReturn::ERROR。
   */
  virtual CallbackReturn on_init(
    const hardware_interface::HardwareComponentInterfaceParams & params);

  /// 为 Hardware Component interface 定义自定义 node 选项。
  /**
   * 在加载 Hardware Component 时，Hardware Component 使用本方法
   * 实例化其 Lifecycle node。
   */
  virtual rclcpp::NodeOptions define_custom_node_options() const;

  /**
   * 覆盖本方法以导出 URDF 文件中未定义的自定义 StateInterface。
   * 这些 interface 将加入 unlisted_state_interfaces_ 映射。
   *
   * \return 未列出 StateInterface 的描述向量
   */
  virtual std::vector<hardware_interface::InterfaceDescription>
  export_unlisted_state_interface_descriptions();

  /**
   * 导出 StateInterface 的默认实现。StateInterface 根据
   * InterfaceDescription 创建。controller 与 hardware 访问的
   * 内存在此分配并驻留在 interface 中。
   *
   * \return 指向已创建并保存的 StateInterface 的 shared pointer 向量
   */
  virtual std::vector<StateInterface::ConstSharedPtr> on_export_state_interfaces();

  /**
   * 覆盖本方法以导出 URDF 文件中未定义的自定义 CommandInterface。
   * 这些 interface 将加入 unlisted_command_interfaces_ 映射。
   *
   * \return 未列出 CommandInterface 的描述向量
   */
  virtual std::vector<hardware_interface::InterfaceDescription>
  export_unlisted_command_interface_descriptions();

  /**
   * 导出 CommandInterface 的默认实现。CommandInterface 根据
   * InterfaceDescription 创建。controller 与 hardware 访问的
   * 内存在此分配并驻留在 system_interface 中。
   *
   * Actuator 与 System 组件应覆盖本方法。Sensor 组件可使用
   * 默认实现。
   *
   * \return 指向已创建并保存的 CommandInterface 的 shared pointer 向量
   */
  virtual std::vector<CommandInterface::SharedPtr> on_export_command_interfaces();

  /// 为新的 Command interface 切换做准备。
  /**
   * 为新的 Command interface 组合所需的任何模式切换做准备。
   *
   * \note 这是对一组 Command interface 声明是否可行的非实时评估，
   * 并调用以开始为即将到来的切换准备数据结构。
   * \param[in] start_interfaces 将要启动的 Command interface 字符串标识符向量。
   * \param[in] stop_interfaces 将要停止的 Command interface 字符串标识符向量。
   * \return 若新的 Command interface 组合可准备好（或）若 interface 键
   * 与本系统无关，则返回 return_type::OK。否则返回 return_type::ERROR。
   */
  virtual return_type prepare_command_mode_switch(
    const std::vector<std::string> & start_interfaces,
    const std::vector<std::string> & stop_interfaces);

  // 执行向新 Command interface 的切换。
  /**
   * 为新的 Command interface 组合执行模式切换。
   *
   * \note 属于实时更新循环的一部分，应快速完成。
   * \param[in] start_interfaces 将要启动的 Command interface 字符串标识符向量。
   * \param[in] stop_interfaces 将要停止的 Command interface 字符串标识符向量。
   * \return 若可切换到新的 Command interface 组合（或）若 interface 键
   * 与本系统无关，则返回 return_type::OK。否则返回 return_type::ERROR。
   */
  virtual return_type perform_command_mode_switch(
    const std::vector<std::string> & start_interfaces,
    const std::vector<std::string> & stop_interfaces);

  /// 根据 HardwareInfo 同步或异步触发 read 方法
  /**
   * 必须更新自物理 hardware 的读数，
   * 并在导出的 State interface 中相应反映。
   * 即 interface 所指向的数据应被更新。
   * 本方法在 ResourceManager 的 read 循环中被调用
   *
   * \param[in] time 本轮控制循环开始时刻
   * \param[in] period 上一轮控制循环测得的耗时
   * \return read 成功返回 return_type::OK，否则返回 return_type::ERROR。
   */
  HardwareComponentCycleStatus trigger_read(
    const rclcpp::Time & time, const rclcpp::Duration & period);

  /// 从 hardware 读取当前 State 值。
  /**
   * 必须更新自物理 hardware 的读数，
   * 并在导出的 State interface 中相应反映。
   * 即 interface 所指向的数据应被更新。
   *
   * \param[in] time 本轮控制循环开始时刻
   * \param[in] period 上一轮控制循环测得的耗时
   * \return read 成功返回 return_type::OK，否则返回 return_type::ERROR。
   */
  virtual return_type read(const rclcpp::Time & time, const rclcpp::Duration & period) = 0;

  /// 根据 HardwareInfo 同步或异步触发 write 方法
  /**
   * 应使用导出的 Command interface 中的最新值
   * 更新物理 hardware。
   * 本方法在 ResourceManager 的 write 循环中被调用
   *
   * \param[in] time 本轮控制循环开始时刻
   * \param[in] period 上一轮控制循环测得的耗时
   * \return write 成功返回 return_type::OK，否则返回 return_type::ERROR。
   */
  HardwareComponentCycleStatus trigger_write(
    const rclcpp::Time & time, const rclcpp::Duration & period);

  /// 将当前 command 值写入 hardware。
  /**
   * 应使用导出的 Command interface 中的最新值
   * 更新物理 hardware。
   *
   * \param[in] time 本轮控制循环开始时刻
   * \param[in] period 上一轮控制循环测得的耗时
   * \return write 成功返回 return_type::OK，否则返回 return_type::ERROR。
   */
  virtual return_type write(const rclcpp::Time & time, const rclcpp::Duration & period);

  /// 获取 hardware 名称。
  /**
   * \return 名称。
   */
  const std::string & get_name() const;

  /// 获取其所属 hardware 组的名称。
  /**
   * \return 组名称。
   */
  const std::string & get_group_name() const;

  /// 获取 hardware 的 Lifecycle 状态。
  /**
   * \note 访问返回的 rclcpp_lifecycle::State 的成员非实时安全，
   * 不应在控制循环中调用。
   * \note 本方法线程安全。
   * \return 状态。
   */
  const rclcpp_lifecycle::State & get_lifecycle_state() const;

  /// 设置 hardware 的 Lifecycle 状态。
  /**
   * 获取 Hardware Component interface 内部缓存的 Lifecycle id，以避免
   * 在实时控制循环中调用 get_lifecycle_state()。
   * \note 本方法实时安全且线程安全，可在控制循环中调用。
   * \return 状态。
   */
  void set_lifecycle_state(const rclcpp_lifecycle::State & new_state);

  /// 获取 Hardware Component interface 的 Lifecycle id。
  uint8_t get_lifecycle_id() const;

  /// State interface 是否存在？
  /**
   * \param[in] interface_name State interface 的名称。
   * \return 存在返回 true，否则返回 false。
   */
  virtual bool has_state(const std::string & interface_name) const;

  /// 获取 State interface 句柄
  /**
   * \param[in] interface_name 要访问的 State interface 名称。
   * \return 指向 State interface 句柄的 Shared pointer。
   * \throws std::runtime_error 若找不到给定名称的 State
   * interface，本方法抛出 runtime_error。
   */
  virtual const StateInterface::SharedPtr & get_state_interface_handle(
    const std::string & interface_name) const;

  /// 设置 State interface 的值。
  /**
   * \tparam T 要存储的值的类型。
   * \param interface_handle 要访问的 State interface 的 shared pointer。
   * \param value 要存储的值。
   * \param wait_until_set 若为 true，本方法确保值被成功设置
   * \note 在 wait_until_set = true 时，本方法非实时安全。
   * \throws std::runtime_error 若无法
   * 访问 State interface，本方法抛出 runtime_error。
   * \return 值设置成功返回 true，否则返回 false。
   */
  template <typename T>
  bool set_state(
    const StateInterface::SharedPtr & interface_handle, const T & value, bool wait_until_set)
  {
    if (!interface_handle)
    {
      throw std::runtime_error(
        fmt::format(
          "State interface handle is null in hardware component: {}, while calling set_state "
          "method. This should not happen.",
          info_.name));
    }
    return interface_handle->set_value(value, wait_until_set);
  }

  /// 设置 State interface 的值。
  /**
   * \tparam T 要存储的值的类型。
   * \param[in] interface_name 要访问的 State interface 名称。
   * \param[in] value 要存储的值。
   * \note 本方法非实时安全。
   * \throws std::runtime_error 若无法
   * 访问 State interface，本方法抛出 runtime_error。
   */
  template <typename T>
  void set_state(const std::string & interface_name, const T & value)
  {
    std::ignore = set_state(get_state_interface_handle(interface_name), value, true);
  }

  /**
   * \tparam T 要读取的值的类型。
   * \param[in] interface_handle 要访问的 State interface 的 shared pointer。
   * \param[out] state 用于存放读取值的变量。
   * \param[in] wait_until_get 若为 true，本方法确保值被成功读取
   * \note 在 wait_until_get = true 时，本方法非实时安全。
   * \throws std::runtime_error 在 wait_until_get 为 true 时，若无法
   * 访问 State interface 或其存储的值，本方法抛出 runtime_error。
   * \return 值读取成功返回 true，否则返回 false。
   */
  template <typename T>
  bool get_state(
    const StateInterface::SharedPtr & interface_handle, T & state, bool wait_until_get) const
  {
    if (!interface_handle)
    {
      throw std::runtime_error(
        fmt::format(
          "State interface handle is null in hardware component: {}, while calling get_state "
          "method. This should not happen.",
          info_.name));
    }
    const bool success = interface_handle->get_value(state, wait_until_get);
    if (!success && wait_until_get)
    {
      throw std::runtime_error(
        fmt::format(
          "Failed to get state value from interface: {} in hardware component: {}. This should "
          "not happen.",
          interface_handle->get_name(), info_.name));
    }
    return success;
  }

  /// 从 State interface 获取值。
  /**
   * \tparam T 要读取的值的类型。
   * \param[in] interface_name 要访问的 State interface 名称。
   * \return 从 interface 得到的值。
   * \note 本方法非实时安全。
   * \throws std::runtime_error 若无法
   * 访问 State interface 或其存储的值，本方法抛出 runtime_error。
   */
  template <typename T = double>
  T get_state(const std::string & interface_name) const
  {
    T state;
    get_state<T>(get_state_interface_handle(interface_name), state, true);
    return state;
  }

  /// Command interface 是否存在？
  /**
   * \param[in] interface_name Command interface 的名称。
   * \return 存在返回 true，否则返回 false。
   */
  virtual bool has_command(const std::string & interface_name) const;

  /// 获取 Command interface 句柄
  /**
   * \param[in] interface_name 要访问的 Command interface 名称。
   * \return 指向 Command interface 句柄的 Shared pointer。
   * \throws std::runtime_error 若找不到给定名称的 Command
   * interface，本方法抛出 runtime_error。
   */
  virtual const CommandInterface::SharedPtr & get_command_interface_handle(
    const std::string & interface_name) const;

  /// 设置 Command interface 的值。
  /**
   * \tparam T 要存储的值的类型。
   * \param interface_handle 要访问的 Command interface 的 shared pointer。
   * \param value 要存储的值。
   * \param wait_until_set 若为 true，本方法确保值被成功设置
   * \note 在 wait_until_set = true 时，本方法非实时安全
   * \throws 若无法访问 Command interface，本方法抛出 runtime_error。
   * \return 值设置成功返回 true，否则返回 false。
   */
  template <typename T>
  bool set_command(
    const CommandInterface::SharedPtr & interface_handle, const T & value, bool wait_until_set)
  {
    if (!interface_handle)
    {
      throw std::runtime_error(
        fmt::format(
          "Command interface handle is null in hardware component: {}, while calling set_command "
          "method. This should not happen.",
          info_.name));
    }
    return interface_handle->set_value(value, wait_until_set);
  }

  /// 设置 Command interface 的值。
  /**
   * \tparam T 要存储的值的类型。
   * \param interface_name 要访问的 command
   * interface 名称。
   * \param value 要存储的值。
   * \note 本方法非实时安全。
   * \throws 若无法访问 Command interface，本方法抛出 runtime_error。
   */
  template <typename T>
  void set_command(const std::string & interface_name, const T & value)
  {
    std::ignore = set_command(get_command_interface_handle(interface_name), value, true);
  }

  /**
   * \tparam T 要读取的值的类型。
   * \param[in] interface_handle 要访问的 Command interface 的 shared pointer。
   * \param[out] command 用于存放读取值的变量。
   * \param[in] wait_until_get 若为 true，本方法确保值被成功读取
   * \note 在 wait_until_get = true 时，本方法非实时安全。
   * \return 值读取成功返回 true，否则返回 false。
   * \throws std::runtime_error 在 wait_until_get 为 true 时，若无法
   * 访问 Command interface 或其存储的值，本方法抛出 runtime_error。
   */
  template <typename T>
  bool get_command(
    const CommandInterface::SharedPtr & interface_handle, T & command, bool wait_until_get) const
  {
    if (!interface_handle)
    {
      throw std::runtime_error(
        fmt::format(
          "Command interface handle is null in hardware component: {}, while calling get_command "
          "method. This should not happen.",
          info_.name));
    }
    const bool success = interface_handle->get_value(command, wait_until_get);
    if (!success && wait_until_get)
    {
      throw std::runtime_error(
        fmt::format(
          "Failed to get command value from interface: {} in hardware component: {}. This should "
          "not happen.",
          interface_handle->get_name(), info_.name));
    }
    return success;
  }

  ///  从 Command interface 获取值。
  /**
   * \tparam T 要读取的值的类型。
   * \param[in] interface_name 要访问的 Command interface 名称。
   * \return 从 interface 得到的值。
   * \note 本方法非实时安全。
   * \throws std::runtime_error 若无法
   * 访问 Command interface 或其存储的值，本方法抛出 runtime_error。
   */
  template <typename T = double>
  T get_command(const std::string & interface_name) const
  {
    T command;
    get_command<T>(get_command_interface_handle(interface_name), command, true);
    return command;
  }

  /// 获取 HardwareComponentInterface 的 logger。
  /**
   * \return HardwareComponentInterface 的 logger。
   */
  virtual rclcpp::Logger get_logger() const;

  /// 获取 clock
  /**
   * \return 与 controller manager 共享的 clock
   */
  virtual rclcpp::Clock::SharedPtr get_clock() const;

  /// 获取 HardwareComponentInterface 的默认 node。
  /**
   * \return HardwareComponentInterface 的 node。
   */
  virtual rclcpp::Node::SharedPtr get_node() const;

  /// 获取 HardwareComponentInterface 的 hardware 信息。
  /**
   * \return HardwareComponentInterface 的 hardware 信息。
   */
  const HardwareInfo & get_hardware_info() const;

  /// 暂停所有异步操作。
  /**
   * 调用本方法以暂停正在进行的异步操作，例如 read/write 周期。
   * 通常在 Lifecycle 转移期间或需要暂停 hardware 时使用。
   */
  void pause_async_operations();

  /// 停止并释放 async handler 线程。
  /**
   * join async 工作线程并销毁 handler。
   */
  void stop_async_handler();

  /// 为激活 hardware 做准备。
  /**
   * 在 ResourceManager 激活 hardware 之前调用本方法。
   */
  void prepare_for_activation();

  /// 启用或禁用 hardware 自省（introspection）。
  /**
   * \param[in] enable 为 true 则启用 introspection，否则禁用。
   */
  void enable_introspection(bool enable);

protected:
  HardwareInfo info_;
  // interface 名称到 InterfaceDescription 的映射
  std::unordered_map<std::string, InterfaceDescription> joint_state_interfaces_;
  std::unordered_map<std::string, InterfaceDescription> joint_command_interfaces_;

  std::unordered_map<std::string, InterfaceDescription> sensor_state_interfaces_;

  std::unordered_map<std::string, InterfaceDescription> gpio_state_interfaces_;
  std::unordered_map<std::string, InterfaceDescription> gpio_command_interfaces_;

  std::unordered_map<std::string, InterfaceDescription> unlisted_state_interfaces_;
  std::unordered_map<std::string, InterfaceDescription> unlisted_command_interfaces_;

  rclcpp_lifecycle::State lifecycle_state_;
  std::unique_ptr<realtime_tools::AsyncFunctionHandler<return_type>> async_handler_;

  // 按 hardware 描述中列出顺序导出的 Command 与 StateInterface
  std::vector<StateInterface::SharedPtr> joint_states_;
  std::vector<CommandInterface::SharedPtr> joint_commands_;

  std::vector<StateInterface::SharedPtr> sensor_states_;

  std::vector<StateInterface::SharedPtr> gpio_states_;
  std::vector<CommandInterface::SharedPtr> gpio_commands_;

  std::vector<StateInterface::SharedPtr> unlisted_states_;
  std::vector<CommandInterface::SharedPtr> unlisted_commands_;

private:
  class HardwareComponentInterfaceImpl;
  std::unique_ptr<HardwareComponentInterfaceImpl> impl_;

protected:
  pal_statistics::RegistrationsRAII stats_registrations_;
};

}  // namespace hardware_interface
#endif  // HARDWARE_INTERFACE__HARDWARE_COMPONENT_INTERFACE_HPP_