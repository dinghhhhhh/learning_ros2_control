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

#ifndef HARDWARE_INTERFACE__COMPONENT_PARSER_HPP_
#define HARDWARE_INTERFACE__COMPONENT_PARSER_HPP_

#include <string>
#include <unordered_map>
#include <vector>

#include "hardware_interface/hardware_info.hpp"

namespace hardware_interface
{
/// 在 URDF 的 XML 片段中查找 control 组件相关信息。
/**
 * \param[in] urdf 包含机器人 URDF 的字符串
 * \return 填充机器人 control 资源信息的 vector
 * \throws 若未找到 robot 属性或标签则抛出 std::runtime_error
 */
std::vector<HardwareInfo> parse_control_resources_from_urdf(const std::string & urdf);

/**
 * \param[in] component_info 组件（gpio、joint、sensor）信息
 * \return 填充该组件已导出 StateInterface 信息的 vector
 */
std::vector<InterfaceDescription> parse_state_interface_descriptions(
  const std::vector<ComponentInfo> & component_info);

/**
 * \param[in] component_info 组件（gpio、joint、sensor）信息
 * \param[out] state_interfaces_map 填充该组件已导出 StateInterface 信息的 unordered_map
 */
void parse_state_interface_descriptions(
  const std::vector<ComponentInfo> & component_info,
  std::unordered_map<std::string, InterfaceDescription> & state_interfaces_map);

/**
 * \param[in] component_info 组件（gpio、joint、sensor）信息
 * \return 填充该组件已导出 CommandInterface 信息的 vector
 */
std::vector<InterfaceDescription> parse_command_interface_descriptions(
  const std::vector<ComponentInfo> & component_info);

/**
 * \param[in] component_info 组件（gpio、joint、sensor）信息
 * \param[out] command_interfaces_map 填充该组件已导出 CommandInterface 信息的 unordered_map
 */
void parse_command_interface_descriptions(
  const std::vector<ComponentInfo> & component_info,
  std::unordered_map<std::string, InterfaceDescription> & command_interfaces_map);
}  // namespace hardware_interface
#endif  // HARDWARE_INTERFACE__COMPONENT_PARSER_HPP_
