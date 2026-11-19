// Copyright (c) 2021, Stogl Robotics Consulting UG (haftungsbeschränkt)
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

//
// Author: Denis Stogl
//

#ifndef HARDWARE_INTERFACE__HARDWARE_COMPONENT_INFO_HPP_
#define HARDWARE_INTERFACE__HARDWARE_COMPONENT_INFO_HPP_

#include <memory>
#include <string>
#include <vector>

#include "rclcpp/time.hpp"
#include "rclcpp_lifecycle/state.hpp"

#include "hardware_interface/types/statistics_types.hpp"
namespace hardware_interface
{
struct HardwareComponentStatisticsData
{
  ros2_control::MovingAverageStatisticsData execution_time;
  ros2_control::MovingAverageStatisticsData periodicity;
};
/// Hardware 组件信息
/**
 * 该结构体包含给定 hardware 组件的信息。
 */
struct HardwareComponentInfo
{
  /// 组件名称。
  std::string name;

  /// 组件「分类」："actuator"、"sensor" 或 "system"
  std::string type;

  /// 组件组
  std::string group;

  /// 组件的 pluginlib 插件名称。
  std::string plugin_name;

  /// 组件是否为 async
  bool is_async;

  //// 读写频率
  unsigned int rw_rate;

  /// 组件当前 Lifecycle 状态。
  rclcpp_lifecycle::State state;

  /// 组件提供的 state interface 列表。
  std::vector<std::string> state_interfaces;

  /// 组件提供的 command interface 列表。
  std::vector<std::string> command_interfaces;

  /// 组件的 read 周期统计。
  std::shared_ptr<HardwareComponentStatisticsData> read_statistics = nullptr;

  /// 组件的 write 周期统计。
  std::shared_ptr<HardwareComponentStatisticsData> write_statistics = nullptr;
};

}  // namespace hardware_interface
#endif  // HARDWARE_INTERFACE__HARDWARE_COMPONENT_INFO_HPP_
