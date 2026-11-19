// Copyright 2020 ros2_control Development Team
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

#ifndef HARDWARE_INTERFACE__TYPES__HARDWARE_INTERFACE_TYPE_VALUES_HPP_
#define HARDWARE_INTERFACE__TYPES__HARDWARE_INTERFACE_TYPE_VALUES_HPP_

namespace hardware_interface
{
/// 定义 position interface 名称的常量
constexpr char HW_IF_POSITION[] = "position";
/// 定义 velocity interface 名称的常量
constexpr char HW_IF_VELOCITY[] = "velocity";
/// 定义 acceleration interface 名称的常量
constexpr char HW_IF_ACCELERATION[] = "acceleration";
/// 定义 effort interface 名称的常量
constexpr char HW_IF_EFFORT[] = "effort";
/// 定义 torque interface 名称的常量
constexpr char HW_IF_TORQUE[] = "torque";
/// 定义 force interface 名称的常量
constexpr char HW_IF_FORCE[] = "force";
/// 定义 current interface 名称的常量
constexpr char HW_IF_CURRENT[] = "current";
/// 定义 temperature interface 名称的常量
constexpr char HW_IF_TEMPERATURE[] = "temperature";

/// Gains interface 相关常量
/// 定义 proportional gain interface 名称的常量
constexpr char HW_IF_PROPORTIONAL_GAIN[] = "proportional";
/// 定义 integral gain interface 名称的常量
constexpr char HW_IF_INTEGRAL_GAIN[] = "integral";
/// 定义 derivative gain interface 名称的常量
constexpr char HW_IF_DERIVATIVE_GAIN[] = "derivative";
/// 定义 integral clamp interface 名称的常量
constexpr char HW_IF_INTEGRAL_CLAMP_MAX[] = "integral_clamp_max";
/// 定义 integral clamp interface 名称的常量
constexpr char HW_IF_INTEGRAL_CLAMP_MIN[] = "integral_clamp_min";
/// 定义 feedforward interface 名称的常量
constexpr char HW_IF_FEEDFORWARD[] = "feedforward";
}  // namespace hardware_interface

#endif  // HARDWARE_INTERFACE__TYPES__HARDWARE_INTERFACE_TYPE_VALUES_HPP_