// Copyright 2020 - 2021 ros2_control Development Team
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

#ifndef HARDWARE_INTERFACE__SYSTEM_INTERFACE_HPP_
#define HARDWARE_INTERFACE__SYSTEM_INTERFACE_HPP_

#include "hardware_interface/hardware_component_interface.hpp"

namespace hardware_interface
{
/**
 * @brief 在将复杂 System 接入 ros2_control 时实现的虚基类。
 *
 * 常见示例为带或不带 sensor 的多 joint 系统，例如工业机器人或人形机器人。
 *
 * 方法返回值的类型为
 * rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn，含义如下：
 *
 * \returns CallbackReturn::SUCCESS 方法执行成功。
 * \returns CallbackReturn::FAILURE 方法执行失败，可再次调用。
 * \returns CallbackReturn::ERROR 发生严重错误，应在
 * "on_error" 方法中处理。
 *
 * 每个方法执行结束后，硬件处于以下状态之一：
 *
 * UNCONFIGURED (on_init, on_cleanup):
 *   硬件已初始化，但尚未开始通信，因此没有 interface 可用。
 *
 * INACTIVE (on_configure, on_deactivate):
 *   已与硬件建立通信并完成配置。
 *   可 read State，但 Command interface 不可用。
 *
 * FINALIZED (on_shutdown):
 *   Hardware interface 已准备好卸载/销毁。
 *   已释放所分配的内存。
 *
 * ACTIVE (on_activate):
 *   硬件供电回路已激活，硬件可运动，例如制动器已释放。
 *   Command interface 可用。
 *
 * \todo
 * 实现
 *  * https://github.com/ros-controls/ros2_control/issues/931
 *  * https://github.com/ros-controls/roadmap/pull/51/files
 *  * 即在 INACTIVE 状态下：
 *      * 可 read State，并可对非运动类 hardware interface 下发 command。
 *      * 用于运动的 hardware interface 将不可用。
 *      * 这些 interface 包括：HW_IF_POSITION、HW_IF_VELOCITY、HW_IF_ACCELERATION 和
 * HW_IF_EFFORT。
 */

class SystemInterface : public HardwareComponentInterface
{
public:
  return_type write(const rclcpp::Time & time, const rclcpp::Duration & period) override = 0;
};

}  // namespace hardware_interface
#endif  // HARDWARE_INTERFACE__SYSTEM_INTERFACE_HPP_