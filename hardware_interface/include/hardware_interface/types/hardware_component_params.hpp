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

#ifndef HARDWARE_INTERFACE__TYPES__HARDWARE_COMPONENT_PARAMS_HPP_
#define HARDWARE_INTERFACE__TYPES__HARDWARE_COMPONENT_PARAMS_HPP_

#include <memory>
#include <string>
#include "hardware_interface/hardware_info.hpp"
#include "rclcpp/rclcpp.hpp"

namespace hardware_interface
{

/**
 * @brief 初始化特定 hardware component 插件所需的参数。
 * 通常用于 hardware component 的 init、initialise 和 load 方法。
 * 用户一般无法直接访问该结构体。
 * 该结构体通常由 ResourceManagerParams 中的数据填充。
 */
struct HardwareComponentParams
{
  HardwareComponentParams() = default;
  /**
   * @brief 指向本组件对应 HardwareInfo 结构体的引用，由 URDF 解析得到。
   * 调用方（例如 ResourceManager）必须保证在本结构体使用期间 HardwareInfo 对象的生命周期。
   */
  hardware_interface::HardwareInfo hardware_info;

  /**
   * @brief 取自 resource manager 的 logger 实例
   */
  rclcpp::Logger logger = rclcpp::get_logger("resource_manager");

  /**
   * @brief 供本 hardware component 使用的 rclcpp::Clock 共享指针。
   * 通常与 ResourceManager/ControllerManager 使用同一 clock。
   */
  rclcpp::Clock::SharedPtr clock = nullptr;

  /**
   * @brief hardware component 内部节点所使用的 namespace。
   * 通常与 controller manager 节点的 namespace 相同。
   */
  std::string node_namespace = "";

  /**
   * @brief 指向 rclcpp::Executor 的 weak 指针。Hardware component
   * 可在 lock 后使用它将自身的内部 ROS 2 节点
   * 加入 ControllerManager 的 executor。
   */
  rclcpp::Executor::WeakPtr executor;
};

}  // namespace hardware_interface

#endif  // HARDWARE_INTERFACE__TYPES__HARDWARE_COMPONENT_PARAMS_HPP_