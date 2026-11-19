// Copyright 2020 PAL Robotics S.L.
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

#ifndef HARDWARE_INTERFACE__CONTROLLER_INFO_HPP_
#define HARDWARE_INTERFACE__CONTROLLER_INFO_HPP_

#include <string>
#include <vector>

namespace hardware_interface
{
/// Controller 信息
/**
 * 该结构体包含给定 controller 的信息。
 */
struct ControllerInfo
{
  /// Controller 名称。
  std::string name;

  /// Controller 类型。
  std::string type;

  /// Controller 参数文件
  std::vector<std::string> parameters_files;

  /// Controller claim 的 interface 列表。
  std::vector<std::string> claimed_interfaces;

  /// 本 controller 失败时要激活的 fallback controller 名称列表。
  std::vector<std::string> fallback_controllers_names;

  /// Controller 节点 options 参数
  std::vector<std::string> node_options_args;
};

}  // namespace hardware_interface
#endif  // HARDWARE_INTERFACE__CONTROLLER_INFO_HPP_
