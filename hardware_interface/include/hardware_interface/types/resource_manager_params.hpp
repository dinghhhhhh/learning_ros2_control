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

#ifndef HARDWARE_INTERFACE__TYPES__RESOURCE_MANAGER_PARAMS_HPP_
#define HARDWARE_INTERFACE__TYPES__RESOURCE_MANAGER_PARAMS_HPP_

#include <memory>
#include <string>
#include "rclcpp/rclcpp.hpp"

namespace hardware_interface
{

/**
 * @brief 构造并初始化 ResourceManager 所需的参数。
 * 该结构体通常由 ControllerManager 填充。
 */
struct ResourceManagerParams
{
  ResourceManagerParams() = default;
  /**
   * @brief 描述机器人 hardware component 的 URDF 字符串。
   * 若 ResourceManager 在构造时无初始 URDF、
   * 且组件稍后或通过其他方式加载，可为空。
   */
  std::string robot_description = "";

  /**
   * @brief ResourceManager 使用的 Clock 共享指针。
   * 通常通过节点的 get_clock() 获取。
   */
  rclcpp::Clock::SharedPtr clock = nullptr;

  /**
   * @brief ResourceManager 使用的 logger 实例。
   * 通常通过节点的 get_logger() 获取。
   */
  rclcpp::Logger logger = rclcpp::get_logger("resource_manager");

  /**
   * @brief ResourceManager 及其组件使用的 namespace。
   * 通常与 controller manager 节点的 namespace 相同。
   */
  std::string node_namespace = "";

  /**
   * @brief ResourceManager 及其组件（包括选择加入的插件）将使用的
   * rclcpp::Executor 共享指针。
   * 通常为 ControllerManager 的主 executor。
   */
  rclcpp::Executor::SharedPtr executor = nullptr;

  /**
   * @brief 是否应在 URDF 中的 hardware component 加载并初始化成功后
   * 自动全部 activate。
   */
  bool activate_all = false;

  /**
   * @brief 若为 true，controller 可 claim 来自 inactive hardware component 的资源。
   * 若为 false，controller 只能 claim 来自 active hardware component 的资源。
   * 此外，当 hardware component 在 read/write 周期返回 DEACTIVATE 时：若为 true，
   * 使用这些 interface 的 controller 将继续运行；若为 false，
   * 使用这些 interface 的 controller 将被 deactivate。
   * @warning 允许在 inactive hardware 下控制出于安全原因不推荐。
   * 仅在确知后果时谨慎使用。
   * @note 该参数可能在后续版本中弃用或移除，请谨慎使用。
   */
  bool allow_controller_activation_with_inactive_hardware = false;

  /**
   * @brief 若为 true，当 hardware component 在 write 周期返回 DEACTIVATE 时，
   * 其名称将包含在返回的 HardwareReadWriteStatus.failed_hardware_names 列表中。
   * 若为 false，此类 hardware component 名称不会出现在该列表中。
   * 在允许 controller 与 inactive hardware component 协同工作时可能有用。
   * @note 该参数可能在后续版本中弃用或移除，请谨慎使用。
   */
  bool return_failed_hardware_names_on_return_deactivate_write_cycle_ = true;

  /**
   * @brief ControllerManager 的 update rate（Hz）。
   * ResourceManager 可用于配置异步 hardware component 或其他时序相关设置。
   */
  unsigned int update_rate = 100;

  /**
   * @brief 若为 true，ResourceManager 将捕获 controller 与 hardware component
   * 各类操作期间抛出的异常；若为 false，异常将向上传播并导致 ResourceManager 崩溃。
   */
  bool handle_exceptions = true;
};

}  // namespace hardware_interface

#endif  // HARDWARE_INTERFACE__TYPES__RESOURCE_MANAGER_PARAMS_HPP_