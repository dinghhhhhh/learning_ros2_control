// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#ifndef HARDWARE_INTERFACE__TYPES__HARDWARE_INTERFACE_RETURN_VALUES_HPP_
#define HARDWARE_INTERFACE__TYPES__HARDWARE_INTERFACE_RETURN_VALUES_HPP_

#include <chrono>
#include <cstdint>
#include <optional>

namespace hardware_interface
{
enum class return_type : std::uint8_t
{
  OK = 0,
  ERROR = 1,
  DEACTIVATE = 2,
};

/**
 * 用于保存 Hardware read 或 write 方法返回状态的结构体。
 * 状态包含本周期是否成功触发、周期方法的执行结果以及方法执行耗时。
 * 该状态用于向 controller_manager 提供反馈。
 * @var successful: 触发成功为 true，否则为 false。
 * @var result: 更新成功则为 return_type::OK，否则为 return_type::ERROR。
 * @var execution_time: update 方法的执行时长。
 */
struct HardwareComponentCycleStatus
{
  bool successful = true;
  return_type result = return_type::OK;
  std::optional<std::chrono::nanoseconds> execution_time = std::nullopt;
};

}  // namespace hardware_interface

#endif  // HARDWARE_INTERFACE__TYPES__HARDWARE_INTERFACE_RETURN_VALUES_HPP_