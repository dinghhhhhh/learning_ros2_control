// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#ifndef HARDWARE_INTERFACE__RESOURCE_MANAGER_HPP_
#define HARDWARE_INTERFACE__RESOURCE_MANAGER_HPP_

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "hardware_interface/actuator.hpp"
#include "hardware_interface/actuator_interface.hpp"
#include "hardware_interface/hardware_component_info.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/loaned_command_interface.hpp"
#include "hardware_interface/loaned_state_interface.hpp"
#include "hardware_interface/sensor.hpp"
#include "hardware_interface/sensor_interface.hpp"
#include "hardware_interface/system.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_component_params.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "hardware_interface/types/resource_manager_params.hpp"
#include "rclcpp/clock.hpp"
#include "rclcpp/duration.hpp"
#include "rclcpp/executor.hpp"
#include "rclcpp/logger.hpp"
#include "rclcpp/node_interfaces/node_clock_interface.hpp"
#include "rclcpp/node_interfaces/node_logging_interface.hpp"
#include "rclcpp/time.hpp"
#include "rclcpp_lifecycle/state.hpp"

namespace hardware_interface
{
class ResourceStorage;
class ControllerManager;

struct HardwareReadWriteStatus
{
  return_type result;
  std::vector<std::string> failed_hardware_names;
};

class ResourceManager
{
public:
  /// ResourceManager 默认构造函数。
  explicit ResourceManager(
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logger_interface);

  /// ResourceManager 默认构造函数。
  explicit ResourceManager(rclcpp::Clock::SharedPtr clock, rclcpp::Logger logger);

  /// ResourceManager 构造函数。
  /**
   * 实现会加载指定 URDF，初始化其中列出的 hardware 组件，
   * 并填充各自的 state 与 command interface。
   *
   * \param[in] urdf 包含 URDF 的字符串。
   * \param[in] activate_all 是否立即激活所有资源；目前仅用于测试。
   * \param[in] update_rate Controller Manager 的更新频率，用于计算 async 组件的调用频率。
   * \param[in] clock_interface CM 节点的 clock interface 引用，用于获取触发 async 组件
   * 及不同读写组件频率的时间。
   * \param[in] logger_interface CM 节点的 logger interface 引用，用于日志。
   */
  explicit ResourceManager(
    const std::string & urdf,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logger_interface,
    bool activate_all = false, const unsigned int update_rate = 100);

  /// ResourceManager 构造函数。
  /**
   * 实现会加载指定 URDF，初始化其中列出的 hardware 组件，
   * 并填充各自的 state 与 command interface。
   *
   * \param[in] urdf 包含 URDF 的字符串。
   * \param[in] activate_all 是否立即激活所有资源；目前仅用于测试。
   * \param[in] update_rate Controller Manager 的更新频率，用于计算 async 组件的调用频率。
   * \param[in] clock CM 节点的 clock 引用，用于获取触发 async 组件及不同读写组件频率的时间。
   * \param[in] logger CM 节点的 logger，用于日志。
   */
  explicit ResourceManager(
    const std::string & urdf, rclcpp::Clock::SharedPtr clock, rclcpp::Logger logger,
    bool activate_all = false, const unsigned int update_rate = 100);

  /// ResourceManager 构造函数。
  /**
   * 使用 ResourceManagerParams 加载指定 URDF，初始化其中列出的 hardware 组件，
   * 并填充各自的 state 与 command interface。
   *
   * \param[in] params 包含 ResourceManager 参数的 ResourceManagerParams。
   * \param[in] load 是否加载并初始化组件；为 false 时不加载任何组件，
   * 仅用给定参数初始化 ResourceManager。
   */
  explicit ResourceManager(const hardware_interface::ResourceManagerParams & params, bool load);

  ResourceManager(const ResourceManager &) = delete;

  virtual ~ResourceManager();

  /// 关闭所有 hardware 组件，不论其当前状态。
  /**
   * 在 Controller Manager 关闭时调用。
   * @return 全部成功关闭为 true，否则为 false。
   */
  bool shutdown_components();

  /// 从给定 URDF 加载资源。
  /**
   * 可用给定 URDF 对 ResourceManager 进行后初始化。
   * 常与默认构造函数配合使用，因首次初始化时可能尚无 URDF。
   *
   * \param[in] params 含 URDF 与 ResourceManager 配置的参数。
   * \returns URDF 校验失败时返回 false。
   */
  virtual bool load_and_initialize_components(
    const hardware_interface::ResourceManagerParams & params);

  /**
   * @brief 从 URDF 导入 joint limiter。
   * @param urdf 包含 URDF 的字符串。
   */
  void import_joint_limiters(const std::string & urdf);

  /**
   * @brief 若已调用 load_and_initialize_components(...)，则返回 true。
   * 允许稍后再加载 URDF，但目前不允许多次调用 load_and_initialize_components（重载/换 URDF）。
   *
   * @return ResourceManager 已成功加载并初始化组件时为 true
   * @return 未加载并初始化任何组件时为 false
   */
  bool are_components_initialized() const;

  /// 根据 key claim state interface。
  /**
   * 在对象存活期间持有该资源；
   * 超出作用域时析构函数释放。
   *
   * \param[in] key 要 claim 的 state interface 字符串标识
   * \return loaned state interface
   */
  LoanedStateInterface claim_state_interface(const std::string & key);

  /// 返回所有已注册的 state interface key。
  /**
   * 从各已加载 hardware 组件收集 key。
   * \return 包含所有已注册 key 的字符串 vector。
   */
  std::vector<std::string> state_interface_keys() const;

  /// 返回所有可用的 state interface key。
  /**
   * 从可用列表收集 key。
   * \return 包含所有可用 state interface 名称的字符串 vector。
   */
  std::vector<std::string> available_state_interfaces() const;

  /// 检查给定 key 下 state interface 是否可用。
  /**
   * \return 可用为 true，否则为 false。
   */
  bool state_interface_is_available(const std::string & name) const;

  /// 获取 state interface 的数据类型。
  /**
   * \param[in] name 要检查的 interface 标识字符串。
   * \return state interface 的数据类型。
   * \throws 若 state interface 不存在则抛出 std::runtime_error。
   */
  std::string get_state_interface_data_type(const std::string & name) const;

  /// 将 controller 导出的 state interface 加入 ResourceManager。
  /**
   * 将导出的 state interface 管理权转移给 ResourceManager 的接口。
   * 在 controller chaining 中，后继 controller 使用这些 state interface，
   * 故应与 hardware 的 state interface 同样管理。
   *
   * \param[in] controller_name 被导入 state interface 的 controller 名称。
   * \param[in] interfaces controller 的 StateInterface 列表。
   */
  void import_controller_exported_state_interfaces(
    const std::string & controller_name, std::vector<StateInterface::ConstSharedPtr> & interfaces);

  /// 获取某 controller 导出的 state interface 列表。
  /**
   * 返回某 controller 已存储的导出 state interface 名称列表。
   *
   * \param[in] controller_name 要查询的 controller。
   * \returns reference interface 名称列表。
   */
  std::vector<std::string> get_controller_exported_state_interface_names(
    const std::string & controller_name);

  /// 将 controller 导出的 state interface 加入可用列表。
  /**
   * 将指定名称 controller 的 state interface 加入可用列表。
   * 应在开启 chaining 模式激活 controller 时调用，使其导出的 state interface
   * 可供 chained 架构中的其他 controller 使用。
   *
   * \param[in] controller_name 其 interface 应变为可用的 controller 名称。
   */
  void make_controller_exported_state_interfaces_available(const std::string & controller_name);

  /// 从可用列表移除 controller 导出的 state interface。
  /**
   * 从可用列表移除指定 controller 的 interface。
   * 应在 controller 停用时调用，因其 reference interface 不再可供其他 controller 使用。
   *
   * \param[in] controller_name 其 interface 应变为不可用的 controller 名称。
   */
  void make_controller_exported_state_interfaces_unavailable(const std::string & controller_name);

  /// 从 ResourceManager 移除 controller 导出的 state interface。
  /**
   * 从 ResourceManager（资源存储）移除导出的 state interface，
   * 并从所有内部 map 与列表中删除。
   *
   * \param[in] controller_name 要从 ResourceManager 删除的 controller 名称。
   */
  void remove_controller_exported_state_interfaces(const std::string & controller_name);

  /// 将 controller 的 reference interface 加入 ResourceManager。
  /**
   * 将 reference interface 管理权转移给 ResourceManager 的接口。
   * 在 controller chaining 中，reference interface 作为前序 controller 的 command interface，
   * 故应与 hardware 的 command interface 同样管理。
   *
   * \param[in] controller_name 被导入 reference interface 的 controller 名称。
   * \param[in] interfaces controller 的 CommandInterface 列表。
   */
  void import_controller_reference_interfaces(
    const std::string & controller_name,
    const std::vector<hardware_interface::CommandInterface::SharedPtr> & interfaces);

  /// 获取某 controller 的 reference interface 列表。
  /**
   * 返回某 controller 已存储的 reference interface 名称列表。
   *
   * \param[in] controller_name 要查询的 controller。
   * \returns reference interface 名称列表。
   */
  std::vector<std::string> get_controller_reference_interface_names(
    const std::string & controller_name);

  /// 将 controller 的 reference interface 加入可用列表。
  /**
   * 将指定 controller 的 interface 加入可用列表。
   * 应在开启 chaining 模式激活 controller 时调用，
   * 使其 reference interface 可供 chained 架构中的其他 controller 使用。
   *
   * \param[in] controller_name 其 interface 应变为可用的 controller 名称。
   */
  void make_controller_reference_interfaces_available(const std::string & controller_name);

  /// 从可用列表移除 controller 的 reference interface。
  /**
   * 从可用列表移除指定 controller 的 interface。
   * 应在 controller 停用时调用。
   *
   * \param[in] controller_name 其 interface 应变为不可用的 controller 名称。
   */
  void make_controller_reference_interfaces_unavailable(const std::string & controller_name);

  /// 从 ResourceManager 移除 controller 的 reference interface。
  /**
   * 从 ResourceManager（资源存储）移除 reference interface，
   * 并从所有内部 map 与列表中删除。
   *
   * \param[in] controller_name 要从 ResourceManager 删除的 controller 名称。
   */
  void remove_controller_reference_interfaces(const std::string & controller_name);

  /// 缓存 hardware 与使用它的 controller 之间的映射
  /**
   * 根据 \p controller_name 所用 interface 查找 controller 与 hardware 的映射并缓存供后续使用。
   *
   * \param[in] controller_name 提供 interface 列表的 controller 名称。
   * \param[in] interfaces \p controller_name 所使用的 interface 列表。
   */
  void cache_controller_to_hardware(
    const std::string & controller_name, const std::vector<std::string> & interfaces);

  /// 返回使用某 hardware 的已缓存 controller。
  /**
   * 返回使用名称为 \p hardware_name 的 hardware 的已缓存 controller 名称列表。
   *
   * \param[in] hardware_name 要查询的 hardware 名称。
   * \returns 依赖 \p hardware_name 的已缓存 controller 名称列表。
   */
  std::vector<std::string> get_cached_controllers_to_hardware(const std::string & hardware_name);

  /// 检查 command interface 是否已被 claim。
  /**
   * 每个 command interface 只能被一个实例 claim。
   * \note state interface 无对等函数；其只读，可被多实例使用。
   *
   * \param[in] key 要检查的 interface 标识字符串。
   * \return 已被 claim 为 true，可用为 false。
   */
  bool command_interface_is_claimed(const std::string & key) const;

  /// 根据 key claim command interface。
  /**
   * 在对象存活期间持有该资源；
   * 超出作用域时析构释放，供他人 claim。
   *
   * \param[in] key 要 claim 的 command interface 字符串标识
   * \return loaned command interface
   */
  LoanedCommandInterface claim_command_interface(const std::string & key);

  /// 返回所有已注册的 command interface key。
  /**
   * 从各已加载 hardware 组件收集 key。
   * \return 包含所有已注册 key 的字符串 vector。
   */
  std::vector<std::string> command_interface_keys() const;

  /// 返回所有可用的 command interface key。
  /**
   * 从可用列表收集 key。
   * \return 包含所有可用 command interface 名称的字符串 vector。
   */
  std::vector<std::string> available_command_interfaces() const;

  /// 检查给定名称下 command interface 是否可用。
  /**
   * \param[in] name 要检查的 interface 标识字符串。
   * \return 可用为 true，否则为 false。
   */
  bool command_interface_is_available(const std::string & interface) const;

  /// 获取 command interface 的数据类型。
  /**
   * \param[in] name 要检查的 interface 标识字符串。
   * \return command interface 的数据类型。
   * \throws 若 command interface 不存在则抛出 std::runtime_error。
   */
  std::string get_command_interface_data_type(const std::string & name) const;

  /// 返回已加载 actuator 组件数量（size_t）。
  /**
   * \return actuator 组件数量。
   */
  size_t actuator_components_size() const;

  /// 返回已加载 sensor 组件数量。
  /**
   * \return sensor 组件数量。
   */
  size_t sensor_components_size() const;

  /// 返回已加载 system 组件数量。
  /**
   * \return system 组件数量（size_t）。
   */
  size_t system_components_size() const;

  /// 导入未在 URDF 中列出的 hardware 组件
  /**
   * 在 URDF 外初始化的组件可在初始化后添加。
   * 仍应提供该组件的 `HardwareInfo`，可从 URDF 字符串解析（最简便）或手动填写。
   *
   * \note 可能使现有 state/command interface 失效，controller 运行时不应调用。
   * \note 若无 hardware_info，须在调用 import 前在外部完成组件配置。
   * \param[in] system 指向 system interface 的指针。
   * \param[in] params 含 hardware 信息等的 HardwareComponentParams。
   */
  void import_component(
    std::unique_ptr<SystemInterface> system, const HardwareComponentParams & params);

  /// 导入未在 URDF 中列出的 hardware 组件
  /**
   * 在 URDF 外初始化的组件可在初始化后添加。
   * 仍应提供该组件的 `HardwareInfo`，可从 URDF 字符串解析（最简便）或手动填写。
   *
   * \note 可能使现有 state/command interface 失效，controller 运行时不应调用。
   * \note 若无 hardware_info，须在调用 import 前在外部完成组件配置。
   * \param[in] actuator 指向 actuator interface 的指针。
   * \param[in] params 含 hardware 信息等的 HardwareComponentParams。
   */
  void import_component(
    std::unique_ptr<ActuatorInterface> actuator, const HardwareComponentParams & params);

  /// 导入未在 URDF 中列出的 hardware 组件
  /**
   * 在 URDF 外初始化的组件可在初始化后添加。
   * 仍应提供该组件的 `HardwareInfo`，可从 URDF 字符串解析（最简便）或手动填写。
   *
   * \note 可能使现有 state/command interface 失效，controller 运行时不应调用。
   * \note 若无 hardware_info，须在调用 import 前在外部完成组件配置。
   * \param[in] sensor 指向 sensor interface 的指针。
   * \param[in] params 含 hardware 信息等的 HardwareComponentParams。
   */
  void import_component(
    std::unique_ptr<SensorInterface> sensor, const HardwareComponentParams & params);

  /// 返回所有组件的状态。
  /**
   * \return hardware 名称与其状态的 map。
   */
  const std::unordered_map<std::string, HardwareComponentInfo> & get_components_status();

  /// 返回硬 joint limits 的 unordered_map。
  /**
   * \return 硬 joint limits 的 unordered_map。
   */
  const std::unordered_map<std::string, joint_limits::JointLimits> & get_hard_joint_limits() const;

  /// 返回软 joint limits 的 unordered_map。
  /**
   * \return 软 joint limits 的 unordered_map。
   */
  const std::unordered_map<std::string, joint_limits::SoftJointLimits> & get_soft_joint_limits()
    const;

  /// 为新的 command interface 模式准备 hardware 组件
  /**
   * 要求 hardware 组件为新的 command interface claim 做准备。
   *
   * \note 用于根据被 claim 的 command interface 切换 hardware interface 控制模式。
   * \note 非实时场景下准备并接受新的 command 资源组合。
   * \note 对所有 actuator 与 system 组件调用 accept_command_resource_claim，
   * hardware interface 默认应返回 hardware_interface::return_type::OK
   * \param[in] start_interfaces 将要启动的 command interface 字符串标识 vector。
   * \param[in] stop_interfaces 将要停止的 command interface 字符串标识 vector。
   * \return 可准备切换为 true；组件拒绝、或任一输入 interface 不存在/不可用
   * （组件非 ACTIVE/INACTIVE）时为 false。
   */
  bool prepare_command_mode_switch(
    const std::vector<std::string> & start_interfaces,
    const std::vector<std::string> & stop_interfaces);

  /// 通知 hardware 组件应进行实时 hardware 模式切换。
  /**
   * 要求 hardware 组件执行 command interface 模式切换。
   *
   * \note 用于根据被 claim 的 command interface 切换控制模式。
   * \note 用于 command interface 的实时切换。
   * \note 假定本方法调用前已用相同参数调用 `prepare_command_mode_switch`。
   * \param[in] start_interfaces 将要启动的 command interface 字符串标识 vector。
   * \param[in] stop_interfaces 将要停止的 command interface 字符串标识 vector。
   * \return 切换成功为 true，组件拒绝则为 false。
   */
  bool perform_command_mode_switch(
    const std::vector<std::string> & start_interfaces,
    const std::vector<std::string> & stop_interfaces);

  /// 设置 hardware 组件的 Lifecycle 状态。
  /**
   * 在可能的情况下设置 hardware 组件状态，
   * 并处理到达目标状态所需的全部转换。
   * 实现见：https://design.ros2.org/articles/node_lifecycle.html
   *
   * 本方法不属于实时关键更新循环。
   *
   * \param[in] component_name 要变更状态的组件名称。
   * \param[in] target_state 目标 Lifecycle 状态。
   * \return 状态切换成功为 hardware_interface::return_type::OK，
   *         任一转换失败为 hardware_interface::return_type::ERROR。
   */
  return_type set_component_state(
    const std::string & component_name, rclcpp_lifecycle::State & target_state);

  /**
   * 对 position、velocity、effort、acceleration interface 强制执行 command limits。
   * @note 本方法 RT-safe
   * @return command 超出 limits 且已强制 limit 时为 true
   * @return command 在 limits 内时为 false
   */
  bool enforce_command_limits(const rclcpp::Duration & period);

  /// 读取所有已加载 hardware 组件。
  /**
   * 从所有 active hardware 组件读取。
   *
   * 属于实时关键更新循环的一部分。
   * hardware interface 实现得当则为 realtime-safe。
   */
  HardwareReadWriteStatus read(const rclcpp::Time & time, const rclcpp::Duration & period);

  /// 写入所有已加载 hardware 组件。
  /**
   * 向所有 active hardware 组件写入。
   *
   * 属于实时关键更新循环的一部分。
   * hardware interface 实现得当则为 realtime-safe。
   */
  HardwareReadWriteStatus write(const rclcpp::Time & time, const rclcpp::Duration & period);

  /// 检查给定 key 下是否已注册 command interface。
  /**
   * \param[in] key 要检查的 interface 标识字符串。
   * \return 存在为 true，否则为 false。
   */
  bool command_interface_exists(const std::string & key) const;

  /// 检查给定 key 下是否已注册 state interface。
  /**
   * \return 存在为 true，否则为 false。
   */
  bool state_interface_exists(const std::string & key) const;

  /// 注册组件状态变化时调用的回调。
  /**
   * \param[in] callback 组件状态变化时调用的函数。
   */
  void set_on_component_state_switch_callback(std::function<void()> callback);

  const std::string & get_robot_description() const;

protected:
  /// 获取 ResourceManager 的 logger
  /**
   * \return ResourceManager 的 logger
   */
  rclcpp::Logger get_logger() const;

  /// 获取 ResourceManager 的 clock
  /**
   * \return ResourceManager 的 clock
   */
  rclcpp::Clock::SharedPtr get_clock() const;

  bool components_are_loaded_and_initialized_ = false;
  bool allow_controller_activation_with_inactive_hardware_ = false;
  bool return_failed_hardware_names_on_return_deactivate_write_cycle_ = true;

  mutable std::recursive_mutex resource_interfaces_lock_;
  mutable std::recursive_mutex claimed_command_interfaces_lock_;
  mutable std::recursive_mutex resources_lock_;
  mutable std::recursive_mutex joint_limiters_lock_;

private:
  bool validate_storage(const std::vector<hardware_interface::HardwareInfo> & hardware_info) const;

  void release_command_interface(const std::string & key);

  // 注：#2323 添加，为与原构造函数向后兼容的临时措施；计划在未来 PR 中连同上述构造函数一并移除。
  hardware_interface::ResourceManagerParams constructParams(
    rclcpp::Clock::SharedPtr clock, rclcpp::Logger logger, const std::string & urdf = std::string(),
    bool activate_all = false, unsigned int update_rate = 100);

  std::unordered_map<std::string, bool> claimed_command_interface_map_;

  std::unique_ptr<ResourceStorage> resource_storage_;
  hardware_interface::ResourceManagerParams params_;

  // 存储 read/write 状态，避免在实时循环中初始化
  HardwareReadWriteStatus read_write_status;
};

}  // namespace hardware_interface
#endif  // HARDWARE_INTERFACE__RESOURCE_MANAGER_HPP_
