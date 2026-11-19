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

#include <fmt/compile.h>
#include <tinyxml2.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "rclcpp/version.h"
// cppcheck-suppress syntaxError
#if RCLCPP_VERSION_GTE(29, 0, 0)
#include "urdf/model.hpp"
#else
#include "urdf/model.h"
#endif

#include "hardware_interface/component_parser.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/helpers.hpp"
#include "hardware_interface/lexical_casts.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "joint_limits/joint_limits_urdf.hpp"

namespace
{
constexpr const auto kRobotTag = "robot";
constexpr const auto kSDFTag = "sdf";
constexpr const auto kModelTag = "model";
constexpr const auto kROS2ControlTag = "ros2_control";
constexpr const auto kHardwareTag = "hardware";
constexpr const auto kPluginNameTag = "plugin";
constexpr const auto kParamTag = "param";
constexpr const auto kGroupTag = "group";
constexpr const auto kActuatorTag = "actuator";
constexpr const auto kJointTag = "joint";
constexpr const auto kSensorTag = "sensor";
constexpr const auto kGPIOTag = "gpio";
constexpr const auto kTransmissionTag = "transmission";
constexpr const auto kCommandInterfaceTag = "command_interface";
constexpr const auto kStateInterfaceTag = "state_interface";
constexpr const auto kMinTag = "min";
constexpr const auto kMaxTag = "max";
constexpr const auto kLimitsTag = "limits";
constexpr const auto kPropertiesTag = "properties";
constexpr const auto kAsyncTag = "async";
constexpr const auto kEnableAttribute = "enable";
constexpr const auto kInitialValueTag = "initial_value";
constexpr const auto kMimicAttribute = "mimic";
constexpr const auto kDataTypeAttribute = "data_type";
constexpr const auto kSizeAttribute = "size";
constexpr const auto kNameAttribute = "name";
constexpr const auto kTypeAttribute = "type";
constexpr const auto kRoleAttribute = "role";
constexpr const auto kReadOnlyAttribute = "read_only";
constexpr const auto kReductionAttribute = "mechanical_reduction";
constexpr const auto kOffsetAttribute = "offset";
constexpr const auto kReadWriteRateAttribute = "rw_rate";
constexpr const auto kIsAsyncAttribute = "is_async";
constexpr const auto kThreadPriorityAttribute = "thread_priority";
constexpr const auto kAffinityCoresAttribute = "affinity";
constexpr const auto kSchedulingPolicyAttribute = "scheduling_policy";
constexpr const auto kPrintWarningsAttribute = "print_warnings";

}  // namespace

namespace hardware_interface
{
namespace detail
{
/// 获取标签之间的文本值。
/**
 * \param[in] element_it 用于查找文本的 XMLElement 迭代器。
 * \param[in] tag_name 查找文本的父标签名（用于错误输出）
 * \return 标签文本，去除首尾空白
 * \throws 未找到文本时抛出 std::runtime_error
 */
std::string get_text_for_element(
  const tinyxml2::XMLElement * element_it, const std::string & tag_name)
{
  const auto get_text_output = element_it->GetText();
  if (!get_text_output)
  {
    std::cerr << "text not specified in the " << tag_name << " tag" << std::endl;
    return "";
  }
  return ros2_control::strip(get_text_output);
}

/// 获取 XMLElement 上 attribute 的值。
/**
 * 若未找到 attribute 则抛出错误。
 *
 * \param[in] element_it 用于查找 attribute 的 XMLElement 迭代器
 * \param[in] attribute_name 要查找并返回的 attribute 名称
 * \param[in] tag_name 查找 attribute 的父标签名（用于错误输出）
 * \return attribute 值，去除首尾空白
 * \throws 未找到 attribute 时抛出 std::runtime_error
 */
std::string get_attribute_value(
  const tinyxml2::XMLElement * element_it, const char * attribute_name, std::string tag_name)
{
  const tinyxml2::XMLAttribute * attr;
  attr = element_it->FindAttribute(attribute_name);
  if (!attr)
  {
    const char * name = element_it->Attribute(kNameAttribute);
    if (name && std::strlen(name) > 0)
    {
      throw std::runtime_error(
        fmt::format(
          FMT_COMPILE("no attribute '{}' in '{}' tag with name '{}'"), attribute_name, tag_name,
          name));
    }
    throw std::runtime_error(
      fmt::format(FMT_COMPILE("no attribute '{}' in '{}' tag"), attribute_name, tag_name));
  }
  return ros2_control::strip(element_it->Attribute(attribute_name));
}

/// 获取 XMLElement 上 attribute 的值。
/**
 * 若未找到 attribute 则抛出错误。
 *
 * \param[in] element_it 用于查找 attribute 的 XMLElement 迭代器
 * \param[in] attribute_name 要查找并返回的 attribute 名称
 * \param[in] tag_name 查找 attribute 的父标签名（用于错误输出）
 * \return attribute 值，去除首尾空白
 * \throws 未找到 attribute 时抛出 std::runtime_error
 */
std::string get_attribute_value(
  const tinyxml2::XMLElement * element_it, const char * attribute_name, const char * tag_name)
{
  return get_attribute_value(element_it, attribute_name, std::string(tag_name));
}

/// 获取 XMLElement 上 parameter 的值。
/**
 * 若未找到 parameter，返回指定的默认值。
 *
 * \param[in] element_it 用于查找 attribute 的 XMLElement 迭代器
 * \param[in] attribute_name 要查找并返回的 attribute 名称
 * \param[in] default_value 未找到 attribute 时返回该值
 * \return attribute 值或默认值
 */
double get_parameter_value_or(
  const tinyxml2::XMLElement * params_it, const char * parameter_name, const double default_value)
{
  while (params_it)
  {
    try
    {
      // 用 parameter 填充 map
      const auto tag_name = params_it->Name();
      if (strcmp(tag_name, parameter_name) == 0)
      {
        const auto tag_text = params_it->GetText();
        if (tag_text)
        {
          return hardware_interface::stod(ros2_control::strip(tag_text));
        }
      }
    }
    catch (const std::exception & e)
    {
      return default_value;
    }

    params_it = params_it->NextSiblingElement();
  }

  return default_value;
}

/// 解析可选的 size attribute
/**
 * 解析 XMLElement 并返回 size attribute 的值。
 * 未指定时默认为 1。若非正整数则抛出错误。
 *
 * \param[in] elem 带有 size attribute 的 XMLElement。
 * \return size。
 * \throws 值不是非零正整数时抛出 std::runtime_error。
 */
std::size_t parse_size_attribute(const tinyxml2::XMLElement * elem)
{
  const tinyxml2::XMLAttribute * attr = elem->FindAttribute(kSizeAttribute);

  if (!attr)
  {
    return 1;
  }

  std::size_t size;
  // 用于校验非零正整数的正则
  std::string s = ros2_control::strip(attr->Value());
  std::regex int_re("[1-9][0-9]*");
  if (std::regex_match(s, int_re))
  {
    size = static_cast<size_t>(std::stoi(s));
  }
  else
  {
    throw std::runtime_error(
      fmt::format(
        FMT_COMPILE(
          "Could not parse size tag in \"{}\". Got \"{}\", but expected a non-zero positive "
          "integer."),
        elem->Name(), s));
  }

  return size;
}

/// 解析 data_type attribute
/**
 * 解析 XMLElement 并返回 data_type attribute 的值。
 * 未指定时默认为 "double"。
 *
 * \param[in] elem 带有 data_type attribute 的 XMLElement。
 * \return 指明数据类型的 string。
 */
std::string parse_data_type_attribute(const tinyxml2::XMLElement * elem)
{
  const tinyxml2::XMLAttribute * attr = elem->FindAttribute(kDataTypeAttribute);
  std::string data_type;
  if (!attr)
  {
    data_type = "double";
  }
  else
  {
    data_type = ros2_control::strip(attr->Value());
  }

  return data_type;
}

/// 解析 rw_rate attribute
/**
 * 解析 XMLElement 并返回 rw_rate attribute 的值。
 * 未指定时默认为 0。
 *
 * \param[in] elem 带有 rw_rate attribute 的 XMLElement。
 * \return 指明 read/write 速率的 unsigned int。
 */
unsigned int parse_rw_rate_attribute(const tinyxml2::XMLElement * elem)
{
  const tinyxml2::XMLAttribute * attr = elem->FindAttribute(kReadWriteRateAttribute);
  try
  {
    const auto rw_rate = attr ? std::stoi(ros2_control::strip(attr->Value())) : 0;
    if (rw_rate < 0)
    {
      throw std::runtime_error(
        fmt::format(
          FMT_COMPILE(
            "Could not parse rw_rate tag in \"{}\". Got \"{}\", but expected a positive integer."),
          elem->Name(), rw_rate));
    }
    return static_cast<unsigned int>(rw_rate);
  }
  catch (const std::invalid_argument & e)
  {
    throw std::runtime_error(
      fmt::format(
        FMT_COMPILE(
          "Could not parse rw_rate tag in \"{}\". Invalid value: \"{}\", expected a positive "
          "integer."),
        elem->Name(), ros2_control::strip(attr->Value())));
  }
  catch (const std::out_of_range & e)
  {
    throw std::runtime_error(
      fmt::format(
        FMT_COMPILE(
          "Could not parse rw_rate tag in \"{}\". Out of range value: \"{}\", expected a positive "
          "valid integer."),
        elem->Name(), ros2_control::strip(attr->Value())));
  }
}

/// 解析 is_async attribute
/**
 * 解析 XMLElement 并返回 is_async attribute 的值。
 * 未指定时默认为 "false"。
 *
 * \param[in] elem 带有 is_async attribute 的 XMLElement。
 * \return 表示读取值为 true 或 false 的 boolean。
 */
bool parse_is_async_attribute(const tinyxml2::XMLElement * elem)
{
  const tinyxml2::XMLAttribute * attr = elem->FindAttribute(kIsAsyncAttribute);
  return attr ? parse_bool(ros2_control::strip(attr->Value())) : false;
}

/// 解析 thread_priority attribute
/**
 * 解析 XMLElement 并返回 thread_priority attribute 的值。
 * 未指定时默认为 50。
 *
 * \param[in] elem 带有 thread_priority attribute 的 XMLElement。
 * \return 指明线程优先级的正整数。
 */
int parse_thread_priority_attribute(const tinyxml2::XMLElement * elem)
{
  const tinyxml2::XMLAttribute * attr = elem->FindAttribute(kThreadPriorityAttribute);
  if (!attr)
  {
    return 50;
  }
  std::string s = ros2_control::strip(attr->Value());
  std::regex int_re("[1-9][0-9]*");
  if (std::regex_match(s, int_re))
  {
    return std::stoi(s);
  }
  else
  {
    throw std::runtime_error(
      fmt::format(
        FMT_COMPILE(
          "Could not parse thread_priority tag in \"{}\". Got \"{}\", but expected a non-zero "
          "positive integer."),
        elem->Name(), s));
  }
}

/// 在 URDF 的 XML 片段中查找 parameter。
/**
 * \param[in] params_it 指向应找到 parameter 信息的迭代器
 * \return parameter 的键值 map
 * \throws 未找到 component attribute 或标签时抛出 std::runtime_error
 */
std::unordered_map<std::string, std::string> parse_parameters_from_xml(
  const tinyxml2::XMLElement * params_it, const std::string & context_name)
{
  std::unordered_map<std::string, std::string> parameters;
  const tinyxml2::XMLAttribute * attr;

  while (params_it)
  {
    // 用 parameter 填充 map
    attr = params_it->FindAttribute(kNameAttribute);
    if (!attr)
    {
      throw std::runtime_error(
        fmt::format(
          FMT_COMPILE("no parameter name attribute set in param tag{}"),
          context_name.empty() ? "" : " for '" + context_name + "'"));
    }
    const std::string parameter_name = ros2_control::strip(params_it->Attribute(kNameAttribute));
    const std::string parameter_value =
      ros2_control::strip(get_text_for_element(params_it, parameter_name));
    parameters[parameter_name] = parameter_value;

    params_it = params_it->NextSiblingElement(kParamTag);
  }
  return parameters;
}

/// 在 XML 片段中查找 interface 类型定义。
/**
 * \param[in] interfaces_it 遍历 interface 的迭代器指针
 * \param[in] interfaceTag interface 类型标签（command 或 state）
 * \return interface 类型列表
 * \throws 标签中未设置 interface 类型文本时抛出 std::runtime_error
 */
hardware_interface::InterfaceInfo parse_interfaces_from_xml(
  const tinyxml2::XMLElement * interfaces_it, const std::string & context_name)
{
  hardware_interface::InterfaceInfo interface;

  const std::string interface_name =
    get_attribute_value(interfaces_it, kNameAttribute, interfaces_it->Name());
  interface.name = interface_name;

  // 可选的 min/max attribute
  std::unordered_map<std::string, std::string> interface_params =
    parse_parameters_from_xml(interfaces_it->FirstChildElement(kParamTag), context_name);
  auto interface_param = interface_params.find(kMinTag);
  if (interface_param != interface_params.end())
  {
    interface.min = interface_param->second;
  }
  interface_param = interface_params.find(kMaxTag);
  if (interface_param != interface_params.end())
  {
    interface.max = interface_param->second;
  }

  // 可选启用或禁用 interface 限位，默认启用
  interface.enable_limits = true;
  const auto * limits_it = interfaces_it->FirstChildElement(kLimitsTag);
  if (limits_it)
  {
    interface.enable_limits =
      parse_bool(get_attribute_value(limits_it, kEnableAttribute, limits_it->Name()));
  }

  // 可选的 initial_value attribute
  interface_param = interface_params.find(kInitialValueTag);
  if (interface_param != interface_params.end())
  {
    interface.initial_value = interface_param->second;
  }

  // 默认为单个 double
  interface.data_type = "double";
  interface.size = 1;

  // 解析 parameter
  const auto * params_it = interfaces_it->FirstChildElement(kParamTag);
  if (params_it)
  {
    interface.parameters = parse_parameters_from_xml(params_it, context_name);
  }

  interface.data_type = parse_data_type_attribute(interfaces_it);
  interface.size = static_cast<int>(parse_size_attribute(interfaces_it));

  return interface;
}

/// 在 URDF 的 XML 片段中查找 control component 信息。
/**
 * \param[in] component_it 指向应找到 component 信息的迭代器
 * \return 填充了 component 信息的 ComponentInfo
 * \throws 未找到 component attribute 或标签时抛出 std::runtime_error
 */
ComponentInfo parse_component_from_xml(const tinyxml2::XMLElement * component_it)
{
  ComponentInfo component;

  // 查找 component 的 name、type
  component.type = component_it->Name();
  component.name = get_attribute_value(component_it, kNameAttribute, component.type);

  if (std::string(kJointTag) == component.type)
  {
    try
    {
      component.is_mimic = parse_bool(get_attribute_value(component_it, kMimicAttribute, kJointTag))
                             ? MimicAttribute::TRUE
                             : MimicAttribute::FALSE;
    }
    catch (const std::runtime_error & e)
    {
      // 未设置 mimic attribute
      component.is_mimic = MimicAttribute::NOT_SET;
    }
  }

  // 可选启用或禁用 interface 限位，默认启用
  component.enable_limits = true;
  const auto * limits_it = component_it->FirstChildElement(kLimitsTag);
  if (limits_it)
  {
    component.enable_limits =
      parse_bool(get_attribute_value(limits_it, kEnableAttribute, limits_it->Name()));
  }

  // 解析所有 command interface
  const auto * command_interfaces_it = component_it->FirstChildElement(kCommandInterfaceTag);
  while (command_interfaces_it)
  {
    InterfaceInfo cmd_info = parse_interfaces_from_xml(command_interfaces_it, component.name);
    cmd_info.enable_limits &= component.enable_limits;
    component.command_interfaces.push_back(cmd_info);
    command_interfaces_it = command_interfaces_it->NextSiblingElement(kCommandInterfaceTag);
  }

  // 解析 state interface
  const auto * state_interfaces_it = component_it->FirstChildElement(kStateInterfaceTag);
  while (state_interfaces_it)
  {
    InterfaceInfo state_info = parse_interfaces_from_xml(state_interfaces_it, component.name);
    state_info.enable_limits &= component.enable_limits;
    component.state_interfaces.push_back(state_info);
    state_interfaces_it = state_interfaces_it->NextSiblingElement(kStateInterfaceTag);
  }

  // 解析 parameter
  const auto * params_it = component_it->FirstChildElement(kParamTag);
  if (params_it)
  {
    component.parameters = parse_parameters_from_xml(params_it, component.name);
  }

  return component;
}

/// 在 URDF 的 XML 片段中查找 complex component 信息。
/**
 * complex component 的 interface 可指定非 double 数据类型，
 * 且 interface 可为该数据类型的固定长度数组。
 *
 * \param[in] component_it 指向应找到 component 信息的迭代器
 * \throws 未找到必需的 component attribute 或标签时抛出 std::runtime_error。
 */
ComponentInfo parse_complex_component_from_xml(const tinyxml2::XMLElement * component_it)
{
  ComponentInfo component;

  // 查找 component 的 name、type
  component.type = component_it->Name();
  component.name = get_attribute_value(component_it, kNameAttribute, component.type);

  // 解析所有 command interface
  const auto * command_interfaces_it = component_it->FirstChildElement(kCommandInterfaceTag);
  while (command_interfaces_it)
  {
    component.command_interfaces.push_back(
      parse_interfaces_from_xml(command_interfaces_it, component.name));
    command_interfaces_it = command_interfaces_it->NextSiblingElement(kCommandInterfaceTag);
  }

  // 解析 state interface
  const auto * state_interfaces_it = component_it->FirstChildElement(kStateInterfaceTag);
  while (state_interfaces_it)
  {
    component.state_interfaces.push_back(
      parse_interfaces_from_xml(state_interfaces_it, component.name));
    state_interfaces_it = state_interfaces_it->NextSiblingElement(kStateInterfaceTag);
  }

  // 解析 parameter
  const auto * params_it = component_it->FirstChildElement(kParamTag);
  if (params_it)
  {
    component.parameters = parse_parameters_from_xml(params_it, component.name);
  }

  return component;
}

JointInfo parse_transmission_joint_from_xml(const tinyxml2::XMLElement * element_it)
{
  JointInfo joint_info;
  joint_info.name = get_attribute_value(element_it, kNameAttribute, element_it->Name());
  joint_info.role = get_attribute_value(element_it, kRoleAttribute, element_it->Name());
  const auto * read_only_attr = element_it->FindAttribute(kReadOnlyAttribute);
  joint_info.read_only =
    read_only_attr ? parse_bool(ros2_control::strip(read_only_attr->Value())) : false;
  joint_info.mechanical_reduction =
    get_parameter_value_or(element_it->FirstChildElement(), kReductionAttribute, 1.0);
  joint_info.offset =
    get_parameter_value_or(element_it->FirstChildElement(), kOffsetAttribute, 0.0);
  return joint_info;
}

ActuatorInfo parse_transmission_actuator_from_xml(const tinyxml2::XMLElement * element_it)
{
  ActuatorInfo actuator_info;
  actuator_info.name = get_attribute_value(element_it, kNameAttribute, element_it->Name());
  actuator_info.role = get_attribute_value(element_it, kRoleAttribute, element_it->Name());
  const auto * read_only_attr = element_it->FindAttribute(kReadOnlyAttribute);
  actuator_info.read_only =
    read_only_attr ? parse_bool(ros2_control::strip(read_only_attr->Value())) : false;
  actuator_info.mechanical_reduction =
    get_parameter_value_or(element_it->FirstChildElement(), kReductionAttribute, 1.0);
  actuator_info.offset =
    get_parameter_value_or(element_it->FirstChildElement(), kOffsetAttribute, 0.0);
  return actuator_info;
}

/// 在 URDF 的 XML 片段中查找 transmission 信息。
/**
 * \param[in] transmission_it 指向应找到 transmission 信息的迭代器
 * \return 填充了 transmission 信息的 TransmissionInfo
 * \throws 未找到 attribute 或标签时抛出 std::runtime_error
 */
TransmissionInfo parse_transmission_from_xml(const tinyxml2::XMLElement * transmission_it)
{
  TransmissionInfo transmission;

  // 查找 transmission 的 name、type
  transmission.name = get_attribute_value(transmission_it, kNameAttribute, transmission_it->Name());
  const auto * type_it = transmission_it->FirstChildElement(kPluginNameTag);
  if (!type_it)
  {
    throw std::runtime_error(
      fmt::format(
        FMT_COMPILE("Missing <plugin> tag of <transmission> element for '{}' in your URDF."),
        transmission.name));
  }
  transmission.type = get_text_for_element(type_it, kPluginNameTag);

  // 解析 joint
  const auto * joint_it = transmission_it->FirstChildElement(kJointTag);
  while (joint_it)
  {
    transmission.joints.push_back(parse_transmission_joint_from_xml(joint_it));
    joint_it = joint_it->NextSiblingElement(kJointTag);
  }

  // 解析 actuator
  const auto * actuator_it = transmission_it->FirstChildElement(kActuatorTag);
  while (actuator_it)
  {
    transmission.actuators.push_back(parse_transmission_actuator_from_xml(actuator_it));
    actuator_it = actuator_it->NextSiblingElement(kActuatorTag);
  }

  // 解析 parameter
  const auto * params_it = transmission_it->FirstChildElement(kParamTag);
  if (params_it)
  {
    transmission.parameters = parse_parameters_from_xml(params_it, transmission.name);
  }

  return transmission;
}

/// 根据上下文自动填充 transmission 信息的部分内容
/**
 * \param[in,out] hardware 已解析各元素的 HardwareInfo 结构体。
 * \throws std::runtime_error
 */
void auto_fill_transmission_interfaces(HardwareInfo & hardware)
{
  for (auto & transmission : hardware.transmissions)
  {
    // 根据本 component 声明的 joint 为 actuator 填充 joint interface
    for (auto & joint : transmission.joints)
    {
      auto found_it = std::find_if(
        hardware.joints.cbegin(), hardware.joints.cend(),
        [&joint](const auto & joint_info) { return joint.name == joint_info.name; });

      if (found_it == hardware.joints.cend())
      {
        throw std::runtime_error(
          fmt::format(
            FMT_COMPILE(
              "Error while parsing '{}'. Transmission '{}' declared joint '{}' is not available in "
              "component '{}'."),
            hardware.name, transmission.name, joint.name, hardware.name));
      }

      // 从 component 中的定义复制 interface 名称
      std::transform(
        found_it->command_interfaces.cbegin(), found_it->command_interfaces.cend(),
        std::back_inserter(joint.command_interfaces),
        [](const auto & interface) { return interface.name; });

      std::transform(
        found_it->state_interfaces.cbegin(), found_it->state_interfaces.cend(),
        std::back_inserter(joint.state_interfaces),
        [](const auto & interface) { return interface.name; });
    }

    // 已解析 actuator component，此处补充更多细节
    if (hardware.type == kActuatorTag)
    {
      if (transmission.joints.size() != 1)
      {
        throw std::runtime_error(
          fmt::format(
            FMT_COMPILE(
              "Error while parsing '{}'. There should be exactly one joint defined in this "
              "component but found {}."),
            hardware.name, transmission.joints.size()));
      }

      transmission.actuators.push_back(
        ActuatorInfo{
          "actuator1", transmission.joints[0].state_interfaces,
          transmission.joints[0].command_interfaces, "actuator1", 1.0, 0.0, false});
    }
  }
}

/// 从 "ros2_control" 标签解析 control resource。
/**
 * \param[in] ros2_control_it 指向含 resource 信息的 ros2_control 元素
 * \return 填充了机器人信息的 HardwareInfo
 * \throws 未找到 attribute 或标签时抛出 std::runtime_error
 */
HardwareInfo parse_resource_from_xml(
  const tinyxml2::XMLElement * ros2_control_it, const std::string & urdf)
{
  HardwareInfo hardware;
  hardware.name = get_attribute_value(ros2_control_it, kNameAttribute, kROS2ControlTag);
  hardware.type = get_attribute_value(ros2_control_it, kTypeAttribute, kROS2ControlTag);
  hardware.rw_rate = parse_rw_rate_attribute(ros2_control_it);
  hardware.is_async = parse_is_async_attribute(ros2_control_it);
  hardware.async_params.thread_priority = hardware.is_async
                                            ? parse_thread_priority_attribute(ros2_control_it)
                                            : std::numeric_limits<int>::max();

  // 解析 ros2_control 标签下的全部内容
  hardware.hardware_plugin_name = "";
  const auto * ros2_control_child_it = ros2_control_it->FirstChildElement();
  while (ros2_control_child_it)
  {
    if (std::string(kHardwareTag) == ros2_control_child_it->Name())
    {
      const auto * type_it = ros2_control_child_it->FirstChildElement(kPluginNameTag);
      if (!type_it)
      {
        throw std::runtime_error(
          fmt::format(
            FMT_COMPILE("Missing <plugin> tag of <hardware> element for '{}' in your URDF."),
            hardware.name));
      }
      hardware.hardware_plugin_name =
        get_text_for_element(type_it, std::string("hardware ") + kPluginNameTag);
      const auto * group_it = ros2_control_child_it->FirstChildElement(kGroupTag);
      if (group_it)
      {
        hardware.group = get_text_for_element(group_it, std::string("hardware.") + kGroupTag);
      }
      const auto * params_it = ros2_control_child_it->FirstChildElement(kParamTag);
      if (params_it)
      {
        hardware.hardware_parameters = parse_parameters_from_xml(params_it, hardware.name);
      }
    }
    else if (std::string(kPropertiesTag) == ros2_control_child_it->Name())
    {
      const auto * async_it = ros2_control_child_it->FirstChildElement(kAsyncTag);
      if (async_it)
      {
        // 已定义 async 属性
        try
        {
          if (async_it->FindAttribute(kAffinityCoresAttribute))
          {
            hardware.async_params.cpu_affinity_cores =
              parse_array<int>(get_attribute_value(async_it, kAffinityCoresAttribute, kAsyncTag));
          }
          if (async_it->FindAttribute(kSchedulingPolicyAttribute))
          {
            hardware.async_params.scheduling_policy =
              to_lower_case(get_attribute_value(async_it, kSchedulingPolicyAttribute, kAsyncTag));
          }
          if (async_it->FindAttribute(kThreadPriorityAttribute))
          {
            hardware.async_params.thread_priority = parse_thread_priority_attribute(async_it);
          }
          if (async_it->FindAttribute(kPrintWarningsAttribute))
          {
            hardware.async_params.print_warnings =
              parse_bool(get_attribute_value(async_it, kPrintWarningsAttribute, kAsyncTag));
          }
        }
        catch (const std::exception & e)
        {
          throw std::runtime_error(
            fmt::format(FMT_COMPILE("Error parsing {} tag: {}"), kAsyncTag, e.what()));
        }
      }
    }
    else if (std::string(kJointTag) == ros2_control_child_it->Name())
    {
      hardware.joints.push_back(parse_component_from_xml(ros2_control_child_it));
    }
    else if (std::string(kSensorTag) == ros2_control_child_it->Name())
    {
      hardware.sensors.push_back(parse_component_from_xml(ros2_control_child_it));
    }
    else if (std::string(kGPIOTag) == ros2_control_child_it->Name())
    {
      hardware.gpios.push_back(parse_complex_component_from_xml(ros2_control_child_it));
    }
    else if (std::string(kTransmissionTag) == ros2_control_child_it->Name())
    {
      hardware.transmissions.push_back(parse_transmission_from_xml(ros2_control_child_it));
    }
    else
    {
      throw std::runtime_error(
        fmt::format(FMT_COMPILE("invalid tag name '{}'"), ros2_control_child_it->Name()));
    }
    ros2_control_child_it = ros2_control_child_it->NextSiblingElement();
  }

  auto_fill_transmission_interfaces(hardware);

  hardware.original_xml = urdf;

  return hardware;
}

/**
 * @brief 从 interface 标签读取 min 与 max 值。
 * @param itf 要读取值的 interface 标签。
 * @param min 要读取的最小值。
 * @param max 要读取的最大值。
 * @return 成功读取返回 true，否则返回 false。
 */
bool retrieve_min_max_interface_values(const InterfaceInfo & itf, double & min, double & max)
{
  try
  {
    if (itf.min.empty() && itf.max.empty())
    {
      // 限位不存在则未读取，返回 false
      return false;
    }
    if (!itf.min.empty())
    {
      min = hardware_interface::stod(itf.min);
    }
    if (!itf.max.empty())
    {
      max = hardware_interface::stod(itf.max);
    }
    return true;
  }
  catch (const std::invalid_argument & err)
  {
    std::cerr << "Error parsing the limits for the interface: " << itf.name << " from the tags ["
              << kMinTag << ": '" << itf.min << "' and " << kMaxTag << ": '" << itf.max
              << "'] within " << kROS2ControlTag << " tag inside the URDF. Skipping it"
              << std::endl;
    return false;
  }
}

/**
 * @brief 设置 acceleration 与 jerk 限位的自定义值（非 URDF 标准）。
 * @param itr 要读取值的 interface 标签。
 * @param limits 要设置的 joint 限位。
 */
void set_custom_interface_values(const InterfaceInfo & itr, joint_limits::JointLimits & limits)
{
  if (itr.name == hardware_interface::HW_IF_ACCELERATION)
  {
    double max_decel(std::numeric_limits<double>::quiet_NaN()),
      max_accel(std::numeric_limits<double>::quiet_NaN());
    if (detail::retrieve_min_max_interface_values(itr, max_decel, max_accel))
    {
      if (std::isfinite(max_decel))
      {
        limits.max_deceleration = std::fabs(max_decel);
        if (!std::isfinite(max_accel))
        {
          limits.max_acceleration = std::fabs(limits.max_deceleration);
        }
        limits.has_deceleration_limits = itr.enable_limits;
      }
      if (std::isfinite(max_accel))
      {
        limits.max_acceleration = max_accel;

        if (!std::isfinite(limits.max_deceleration))
        {
          limits.max_deceleration = std::fabs(limits.max_acceleration);
        }
        limits.has_acceleration_limits = itr.enable_limits;
      }
    }
  }
  else if (itr.name == "jerk")
  {
    if (!itr.min.empty())
    {
      std::cerr << "Error parsing the limits for the interface: " << itr.name
                << " from the tag: " << kMinTag << " within " << kROS2ControlTag
                << " tag inside the URDF. Jerk only accepts max limits." << std::endl;
    }
    double min_jerk(std::numeric_limits<double>::quiet_NaN()),
      max_jerk(std::numeric_limits<double>::quiet_NaN());
    if (
      !itr.max.empty() && detail::retrieve_min_max_interface_values(itr, min_jerk, max_jerk) &&
      std::isfinite(max_jerk))
    {
      limits.max_jerk = std::abs(max_jerk);
      limits.has_jerk_limits = itr.enable_limits;
    }
  }
  else
  {
    if (!itr.min.empty() || !itr.max.empty())
    {
      std::cerr << "Unable to parse the limits for the interface: " << itr.name
                << " from the tags [" << kMinTag << " and " << kMaxTag << "] within "
                << kROS2ControlTag
                << " tag inside the URDF. Supported interfaces for joint limits are: "
                   "position, velocity, effort, acceleration and jerk."
                << std::endl;
    }
  }
}

/**
 * @brief 从 ros2_control command interface 标签读取限位，若更严格则覆盖 URDF 限位。
 * @param joint 包含 interface 及 joint 级 enable_limits 的 joint component 信息。
 * @param limits 要设置的 joint 限位。
 */
void update_interface_limits(const ComponentInfo & joint, joint_limits::JointLimits & limits)
{
  // 若在 joint 级别禁用限位，则禁用所有限位标志
  if (!joint.enable_limits)
  {
    limits.disable_all_limits();
    return;
  }
  for (auto & itr : joint.command_interfaces)
  {
    if (itr.name == hardware_interface::HW_IF_POSITION)
    {
      limits.min_position = limits.has_position_limits && itr.enable_limits
                              ? limits.min_position
                              : -std::numeric_limits<double>::max();
      limits.max_position = limits.has_position_limits && itr.enable_limits
                              ? limits.max_position
                              : std::numeric_limits<double>::max();
      double min_pos(limits.min_position), max_pos(limits.max_position);
      if (itr.enable_limits && detail::retrieve_min_max_interface_values(itr, min_pos, max_pos))
      {
        limits.min_position = std::max(min_pos, limits.min_position);
        limits.max_position = std::min(max_pos, limits.max_position);
        limits.has_position_limits = true;
      }
      limits.has_position_limits &= itr.enable_limits;
    }
    else if (itr.name == hardware_interface::HW_IF_VELOCITY)
    {
      limits.max_velocity =
        limits.has_velocity_limits ? limits.max_velocity : std::numeric_limits<double>::max();
      // 取更严格的一方
      double min_vel(-limits.max_velocity), max_vel(limits.max_velocity);
      if (itr.enable_limits && detail::retrieve_min_max_interface_values(itr, min_vel, max_vel))
      {
        max_vel = std::min(std::abs(min_vel), max_vel);
        limits.max_velocity = std::min(max_vel, limits.max_velocity);
        limits.has_velocity_limits = true;
      }
      limits.has_velocity_limits &= itr.enable_limits;
    }
    else if (itr.name == hardware_interface::HW_IF_EFFORT)
    {
      limits.max_effort =
        limits.has_effort_limits ? limits.max_effort : std::numeric_limits<double>::max();
      // 取更严格的一方
      double min_eff(-limits.max_effort), max_eff(limits.max_effort);
      if (itr.enable_limits && detail::retrieve_min_max_interface_values(itr, min_eff, max_eff))
      {
        max_eff = std::min(std::abs(min_eff), max_eff);
        limits.max_effort = std::min(max_eff, limits.max_effort);
        limits.has_effort_limits = true;
      }
      limits.has_effort_limits &= itr.enable_limits;
    }
    else
    {
      detail::set_custom_interface_values(itr, limits);
    }
  }
}

}  // namespace detail

std::vector<HardwareInfo> parse_control_resources_from_urdf(const std::string & urdf)
{
  // 检查 URDF 字符串是否正常
  if (urdf.empty())
  {
    throw std::runtime_error("empty URDF passed to robot");
  }
  tinyxml2::XMLDocument doc;
  if (!doc.Parse(urdf.c_str()) && doc.Error())
  {
    throw std::runtime_error(
      fmt::format(FMT_COMPILE("invalid URDF passed in to robot parser: {}"), doc.ErrorStr()));
  }
  if (doc.Error())
  {
    throw std::runtime_error(
      fmt::format(FMT_COMPILE("invalid URDF passed in to robot parser: {}"), doc.ErrorStr()));
  }

  // 查找 robot 或 sdf 标签
  const tinyxml2::XMLElement * robot_it = doc.RootElement();
  const tinyxml2::XMLElement * ros2_control_it;

  if (std::string(kRobotTag) == robot_it->Name())
  {
    ros2_control_it = robot_it->FirstChildElement(kROS2ControlTag);
  }
  else if (std::string(kSDFTag) == robot_it->Name())
  {
    // 在 sdf 标签中查找 model 标签
    const tinyxml2::XMLElement * model_it = robot_it->FirstChildElement(kModelTag);
    ros2_control_it = model_it->FirstChildElement(kROS2ControlTag);
  }
  else
  {
    throw std::runtime_error(
      "the robot tag is not root element in URDF or sdf tag is not root element in SDF");
  }

  if (!ros2_control_it)
  {
    throw std::runtime_error(
      fmt::format(FMT_COMPILE("no '{}' tag found in the URDF"), kROS2ControlTag));
  }

  std::vector<HardwareInfo> hardware_info;
  while (ros2_control_it)
  {
    hardware_info.push_back(detail::parse_resource_from_xml(ros2_control_it, urdf));
    ros2_control_it = ros2_control_it->NextSiblingElement(kROS2ControlTag);
  }

  // 完整解析 URDF 以获取 mimic 选项
  urdf::Model model;
  if (!model.initString(urdf))
  {
    throw std::runtime_error("Failed to parse URDF file");
  }
  for (auto & hw_info : hardware_info)
  {
    for (auto i = 0u; i < hw_info.joints.size(); ++i)
    {
      const auto & joint = hw_info.joints.at(i);
      auto urdf_joint = model.getJoint(joint.name);
      if (!urdf_joint)
      {
        throw std::runtime_error(
          fmt::format(FMT_COMPILE("Joint '{}' not found in URDF"), joint.name));
      }
      if (!urdf_joint->mimic && joint.is_mimic == MimicAttribute::TRUE)
      {
        throw std::runtime_error(
          fmt::format(FMT_COMPILE("Joint '{}' has no mimic information in the URDF"), joint.name));
      }
      if (urdf_joint->mimic && joint.is_mimic != MimicAttribute::FALSE)
      {
        if (joint.command_interfaces.size() > 0)
        {
          throw std::runtime_error(
            fmt::format(
              FMT_COMPILE(
                "Joint '{}' has mimic attribute not set to false: Activated mimic joints cannot "
                "have command interfaces."),
              joint.name));
        }
        auto find_joint = [&hw_info](const std::string & name)
        {
          auto it = std::find_if(
            hw_info.joints.begin(), hw_info.joints.end(),
            [&name](const auto & j) { return j.name == name; });
          if (it == hw_info.joints.end())
          {
            throw std::runtime_error(
              fmt::format(FMT_COMPILE("Mimic joint '{}' not found in <ros2_control> tag"), name));
          }
          return static_cast<size_t>(std::distance(hw_info.joints.begin(), it));
        };

        MimicJoint mimic_joint;
        mimic_joint.joint_index = i;
        mimic_joint.mimicked_joint_index = find_joint(urdf_joint->mimic->joint_name);
        mimic_joint.multiplier = urdf_joint->mimic->multiplier;
        mimic_joint.offset = urdf_joint->mimic->offset;
        hw_info.mimic_joints.push_back(mimic_joint);
      }

      if (urdf_joint->type == urdf::Joint::FIXED)
      {
        throw std::runtime_error(
          fmt::format(
            FMT_COMPILE(
              "Joint '{}' is of type 'fixed'. Fixed joints do not make sense in ros2_control."),
            joint.name));
      }

      joint_limits::JointLimits limits;
      getJointLimits(urdf_joint, limits);
      // 取最严格限位；对 continuous joint 类型同样适用
      detail::update_interface_limits(joint, limits);
      hw_info.limits[joint.name] = limits;
      joint_limits::SoftJointLimits soft_limits;
      if (getSoftJointLimits(urdf_joint, soft_limits))
      {
        if (limits.has_position_limits)
        {
          soft_limits.min_position = std::max(soft_limits.min_position, limits.min_position);
          soft_limits.max_position = std::min(soft_limits.max_position, limits.max_position);
        }
        hw_info.soft_limits[joint.name] = soft_limits;
      }
    }
  }

  return hardware_info;
}

std::vector<InterfaceDescription> parse_state_interface_descriptions(
  const std::vector<ComponentInfo> & component_info)
{
  std::vector<InterfaceDescription> component_state_interface_descriptions;
  component_state_interface_descriptions.reserve(component_info.size());

  for (const auto & component : component_info)
  {
    for (const auto & state_interface : component.state_interfaces)
    {
      component_state_interface_descriptions.emplace_back(
        InterfaceDescription(component.name, state_interface));
    }
  }
  return component_state_interface_descriptions;
}

void parse_state_interface_descriptions(
  const std::vector<ComponentInfo> & component_info,
  std::unordered_map<std::string, InterfaceDescription> & state_interfaces_map)
{
  state_interfaces_map.reserve(state_interfaces_map.size() + component_info.size());

  for (const auto & component : component_info)
  {
    for (const auto & state_interface : component.state_interfaces)
    {
      InterfaceDescription description(component.name, state_interface);
      state_interfaces_map.insert(std::make_pair(description.get_name(), description));
    }
  }
}

std::vector<InterfaceDescription> parse_command_interface_descriptions(
  const std::vector<ComponentInfo> & component_info)
{
  std::vector<InterfaceDescription> component_command_interface_descriptions;
  component_command_interface_descriptions.reserve(component_info.size());

  for (const auto & component : component_info)
  {
    for (const auto & command_interface : component.command_interfaces)
    {
      component_command_interface_descriptions.emplace_back(
        InterfaceDescription(component.name, command_interface));
    }
  }
  return component_command_interface_descriptions;
}

void parse_command_interface_descriptions(
  const std::vector<ComponentInfo> & component_info,
  std::unordered_map<std::string, InterfaceDescription> & command_interfaces_map)
{
  command_interfaces_map.reserve(command_interfaces_map.size() + component_info.size());

  for (const auto & component : component_info)
  {
    for (const auto & command_interface : component.command_interfaces)
    {
      InterfaceDescription description(component.name, command_interface);
      command_interfaces_map.insert(std::make_pair(description.get_name(), description));
    }
  }
}

}  // namespace hardware_interface
