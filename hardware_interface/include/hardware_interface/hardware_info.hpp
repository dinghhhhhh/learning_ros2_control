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

#ifndef HARDWARE_INTERFACE__HARDWARE_INFO_HPP_
#define HARDWARE_INTERFACE__HARDWARE_INFO_HPP_

#include <fmt/compile.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "joint_limits/joint_limits.hpp"

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

namespace hardware_interface
{
/**
 * 该结构体存储机器人 URDF 中为特定 hardware 定义的组件信息。
 */
struct InterfaceInfo
{
  /**
   * 可设置的 command interface 名称，例如 "position"、"velocity" 等。
   * 供 joint 与 GPIO 使用。
   */
  std::string name;
  /// （可选）interface 允许的最小值。
  std::string min = "";
  /// （可选）interface 允许的最大值。
  std::string max = "";
  /// （可选）interface 的初始值。
  std::string initial_value = "";
  /// （可选）interface 的数据类型，例如 "bool"、"int"。
  std::string data_type = "double";
  /// （可选）若 Handle 为数组，则为数组大小。
  int size;
  /// （可选）command/state Interface 参数的键值对。
  /// 适用于 modbus 等协议驱动，每个 interface 需要独立的地址（寄存器）、数据类型等。
  std::unordered_map<std::string, std::string> parameters;
  /// （可选）为 command interface 启用或禁用 limits
  bool enable_limits;
};

/// @brief 该结构体存储模仿另一 joint 的 joint 信息
struct MimicJoint
{
  std::size_t joint_index;
  std::size_t mimicked_joint_index;
  double multiplier = 1.0;
  double offset = 0.0;
};

/// @brief 该枚举用于存储 joint 的 mimic 属性
enum class MimicAttribute
{
  NOT_SET,
  TRUE,
  FALSE
};

/**
 * 该结构体存储机器人 URDF 中为特定 hardware 定义的组件信息。
 */
struct ComponentInfo
{
  /// 组件名称。
  std::string name;
  /// 组件类型：sensor、joint 或 GPIO。
  std::string type;

  /// 若给定 mimic 属性则保存其值，否则为 NOT_SET
  MimicAttribute is_mimic = MimicAttribute::NOT_SET;

  /// 该组件是否启用 limits（在 joint 级别通过 <limits enable="..."/> 设置）
  bool enable_limits = true;

  /**
   * 可设置的 command interface 名称，例如 "position"、"velocity" 等。
   * 供 joint 与 GPIO 使用。
   */
  std::vector<InterfaceInfo> command_interfaces;
  /**
   * 可读取的 state interface 名称，例如 "position"、"velocity" 等。
   * 供 joint、sensor 与 GPIO 使用。
   */
  std::vector<InterfaceInfo> state_interfaces;
  /// （可选）组件参数的键值对，例如 min/max 或序列号。
  std::unordered_map<std::string, std::string> parameters;
};

/// 包含自 URDF 加载、用于 transmission 的给定 joint 的语义信息
struct JointInfo
{
  std::string name;
  std::vector<std::string> state_interfaces;
  std::vector<std::string> command_interfaces;
  std::string role;
  double mechanical_reduction = 1.0;
  double offset = 0.0;
  bool read_only = false;
};

/// 包含自 URDF 加载、用于 transmission 的给定 actuator 的语义信息
struct ActuatorInfo
{
  std::string name;
  std::vector<std::string> state_interfaces;
  std::vector<std::string> command_interfaces;
  std::string role;
  double mechanical_reduction = 1.0;
  double offset = 0.0;
  bool read_only = false;
};

/// 包含自 URDF 加载的给定 transmission 的语义信息
struct TransmissionInfo
{
  std::string name;
  std::string type;
  std::vector<JointInfo> joints;
  std::vector<ActuatorInfo> actuators;
  /// （可选）自定义参数的键值对
  std::unordered_map<std::string, std::string> parameters;
};

/**
 * Hardware Handle 支持的类型
 */

using HANDLE_DATATYPE = std::variant<
  std::monostate, double, float, bool, uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t>;
class HandleDataType
{
public:
  enum Value : int8_t
  {
    UNKNOWN = -1,
    DOUBLE,
    FLOAT32,
    BOOL,
    UINT8,
    INT8,
    UINT16,
    INT16,
    UINT32,
    INT32,
  };

  HandleDataType() = default;
  constexpr HandleDataType(Value value) : value_(value) {}  // NOLINT(runtime/explicit)
  explicit HandleDataType(const std::string & data_type)
  {
    if (data_type == "double")
    {
      value_ = DOUBLE;
    }
    else if (data_type == "bool")
    {
      value_ = BOOL;
    }
    else if (data_type == "float32")
    {
      value_ = FLOAT32;
    }
    else if (data_type == "uint8")
    {
      value_ = UINT8;
    }
    else if (data_type == "int8")
    {
      value_ = INT8;
    }
    else if (data_type == "uint16")
    {
      value_ = UINT16;
    }
    else if (data_type == "int16")
    {
      value_ = INT16;
    }
    else if (data_type == "uint32")
    {
      value_ = UINT32;
    }
    else if (data_type == "int32")
    {
      value_ = INT32;
    }
    else
    {
      value_ = UNKNOWN;
    }
  }

  operator Value() const { return value_; }

  explicit operator bool() const = delete;

  constexpr bool operator==(HandleDataType other) const { return value_ == other.value_; }
  constexpr bool operator!=(HandleDataType other) const { return value_ != other.value_; }

  constexpr bool operator==(Value other) const { return value_ == other; }
  constexpr bool operator!=(Value other) const { return value_ != other; }

  std::string to_string() const
  {
    switch (value_)
    {
      case DOUBLE:
        return "double";
      case BOOL:
        return "bool";
      case FLOAT32:
        return "float32";
      case UINT8:
        return "uint8";
      case INT8:
        return "int8";
      case UINT16:
        return "uint16";
      case INT16:
        return "int16";
      case UINT32:
        return "uint32";
      case INT32:
        return "int32";
      default:
        return "unknown";
    }
  }

  /**
   * @brief 检查 HandleDataType 是否可转换为 double。
   * @return 若可转换为 double 则为 true，否则为 false。
   * @note 增加更多数据类型支持后应更新此函数
   */
  bool is_castable_to_double() const
  {
    switch (value_)
    {
      case DOUBLE:   // fallthrough
      case FLOAT32:  // fallthrough
      case BOOL:     // fallthrough
      case UINT8:    // fallthrough
      case INT8:     // fallthrough
      case UINT16:   // fallthrough
      case INT16:    // fallthrough
      case UINT32:   // fallthrough
      case INT32:
        return true;
      default:
        return false;  // 未知类型无法转换
    }
  }

  /**
   * @brief 将给定值转换为 double。
   * @param value 待转换的值。
   * @return 转换后的值。
   * @throw 若 HandleDataType 无法转换为 double 则抛出 std::runtime_error。
   * @note 增加更多数据类型支持后应更新此函数
   */
  double cast_to_double(const HANDLE_DATATYPE & value) const
  {
    switch (value_)
    {
      case DOUBLE:
        return std::get<double>(value);
      case FLOAT32:
        return static_cast<double>(std::get<float>(value));
      case BOOL:
        return static_cast<double>(std::get<bool>(value));
      case UINT8:
        return static_cast<double>(std::get<uint8_t>(value));
      case INT8:
        return static_cast<double>(std::get<int8_t>(value));
      case UINT16:
        return static_cast<double>(std::get<uint16_t>(value));
      case INT16:
        return static_cast<double>(std::get<int16_t>(value));
      case UINT32:
        return static_cast<double>(std::get<uint32_t>(value));
      case INT32:
        return static_cast<double>(std::get<int32_t>(value));
      default:
        throw std::runtime_error(
          fmt::format(FMT_COMPILE("Data type : '{}' cannot be casted to double."), to_string()));
    }
  }

  HandleDataType from_string(const std::string & data_type) { return HandleDataType(data_type); }

private:
  Value value_ = UNKNOWN;
};

/**
 * 该结构体存储应在内部实例化的、特定 hardware 的 interface 信息。
 */
struct InterfaceDescription
{
  InterfaceDescription(const std::string & prefix_name_in, const InterfaceInfo & interface_info_in)
  : prefix_name(prefix_name_in),
    interface_info(interface_info_in),
    interface_name(prefix_name + "/" + interface_info.name)
  {
  }

  /**
   * 用户定义的 interface 前缀名称。
   */
  std::string prefix_name;

  /**
   * Interface 类型（position、velocity 等）及 limits 等信息。
   */
  InterfaceInfo interface_info;

  /**
   * interface 的完整名称
   */
  std::string interface_name;

  const std::string & get_prefix_name() const { return prefix_name; }

  const std::string & get_interface_name() const { return interface_info.name; }

  const std::string & get_name() const { return interface_name; }

  const std::string & get_data_type_string() const { return interface_info.data_type; }

  HandleDataType get_data_type() const { return HandleDataType(interface_info.data_type); }
};

struct HardwareAsyncParams
{
  /// 异步工作线程的线程优先级
  int thread_priority = 50;
  /// 异步工作线程的调度策略
  std::string scheduling_policy = "synchronized";
  /// 异步工作线程的 CPU 亲和性核心
  std::vector<int> cpu_affinity_cores = {};
  /// 异步线程未满足截止时间时是否打印警告
  bool print_warnings = true;
};

/// 该结构体存储机器人 URDF 中定义的 hardware 信息。
struct HardwareInfo
{
  /// hardware 名称。
  std::string name;
  /// hardware 类型：actuator、sensor 或 system。
  std::string type;
  /// hardware 所属的 hardware 组。
  std::string group;
  /// 组件读写频率（Hz）。
  unsigned int rw_rate;
  /// 组件是否为 async
  bool is_async;
  /// 异步参数
  HardwareAsyncParams async_params;
  /// 将要加载的 hardware 的 pluginlib 插件名称。
  std::string hardware_plugin_name;
  /// （可选）hardware 参数的键值对。
  std::unordered_map<std::string, std::string> hardware_parameters;
  /**
   * hardware 提供的 joint 列表。
   * Actuator 与 System Hardware 必需。
   */
  std::vector<ComponentInfo> joints;
  /**
   * mimic joint 列表。
   */
  std::vector<MimicJoint> mimic_joints;
  /**
   * hardware 提供的 sensor 列表。
   * Sensor Hardware 必需，System Hardware 可选。
   */
  std::vector<ComponentInfo> sensors;
  /**
   * hardware 提供的 GPIO 列表。
   * 对所有 hardware 组件均为可选。
   */
  std::vector<ComponentInfo> gpios;
  /**
   * 用于计算 joint 与物理 actuator 之间传动比的 transmission 列表。
   * 对 Actuator 与 System Hardware 可选。
   */
  std::vector<TransmissionInfo> transmissions;
  /**
   * 解析前的 XML 内容
   */
  std::string original_xml;
  /**
   * 自 URDF 解析得到的 hardware 组件 joint command interface 的 limits
   */
  std::unordered_map<std::string, joint_limits::JointLimits> limits;

  /**
   * 用于对 command 进行 clamp 的软件 joint limits 映射，键为 joint 名称。
   * 可选；若未指定或未比 JointLimits 更严格，则沿用先前的 JointLimits。
   */
  std::unordered_map<std::string, joint_limits::SoftJointLimits> soft_limits;
};

}  // namespace hardware_interface
#endif  // HARDWARE_INTERFACE__HARDWARE_INFO_HPP_
