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

#ifndef HARDWARE_INTERFACE__TYPES__HARDWARE_COMPONENT_INTERFACE_PARAMS_HPP_
#define HARDWARE_INTERFACE__TYPES__HARDWARE_COMPONENT_INTERFACE_PARAMS_HPP_

#include <memory>
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/types/hardware_component_params.hpp"
#include "rclcpp/rclcpp.hpp"

namespace hardware_interface
{

/**
 * @brief 初始化特定 hardware component 插件所需的参数。
 * 通常用于 hardware interface 的 on_init 方法，由用户解析。
 * 该结构体通常由各组件的 HardwareComponentParams 填充。
 */
struct HardwareComponentInterfaceParams
{
  HardwareComponentInterfaceParams() = default;
  /**
   * @brief 指向本组件对应 HardwareInfo 结构体的引用，由 URDF 解析得到。
   * 调用方（例如 ResourceManager）必须保证在本结构体使用期间 HardwareInfo 对象的生命周期。
   */
  hardware_interface::HardwareInfo hardware_info;

  /**
   * @brief 指向 rclcpp::Executor 的 weak 指针。Hardware component
   * 可在 lock 后使用它将自身的内部 ROS 2 节点
   * 加入 ControllerManager 的 executor。
   */
  rclcpp::Executor::WeakPtr executor;
};

}  // namespace hardware_interface

#endif  // HARDWARE_INTERFACE__TYPES__HARDWARE_COMPONENT_INTERFACE_PARAMS_HPP_