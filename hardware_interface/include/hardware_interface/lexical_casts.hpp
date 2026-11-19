// Copyright 2023 ros2_control Development Team
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

#ifndef HARDWARE_INTERFACE__LEXICAL_CASTS_HPP_
#define HARDWARE_INTERFACE__LEXICAL_CASTS_HPP_

#include <cstdint>
#include <limits>
#include <regex>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace hardware_interface
{

/** \brief 以与 locale 无关的方式将 std::string 转为 double 的辅助函数。
 * \throws 若非有效数字或超出范围则抛出 std::invalid_argument
 * 摘自
 https://github.com/ros-planning/srdfdom/blob/ad17b8d25812f752c397a6011cec64aeff090c46/src/model.cpp#L53
*/
double stod(const std::string & s);

/** \brief 以与 locale 无关的方式将 std::string 转为 float 的辅助函数。
 * \throws 若非有效数字或超出范围则抛出 std::invalid_argument
 */
float stof(const std::string & s);

/** \brief 从字符串到 int32_t 的防溢出转换。
 * \throws 若转换值超出 int32_t 范围则抛出 std::out_of_range
 * \throws 若无法完成转换则抛出 std::invalid_argument
 */
template <typename T>
T stoi_generic(const std::string & s)
{
  static_assert(std::is_integral_v<T> && std::is_signed_v<T>, "T must be a signed integral type");

  size_t pos;
  const auto v = std::stol(s, &pos);  // stol returns long
  if (pos != s.length())
  {
    throw std::invalid_argument("Invalid characters in string");
  }
  if (v < std::numeric_limits<T>::min() || v > std::numeric_limits<T>::max())
  {
    throw std::out_of_range("value not in range of target type");
  }

  return static_cast<T>(v);
}

// 常用有符号整数类型的显式实例化
inline int8_t stoi8(const std::string & s) { return stoi_generic<int8_t>(s); }
inline int16_t stoi16(const std::string & s) { return stoi_generic<int16_t>(s); }
inline int32_t stoi32(const std::string & s) { return stoi_generic<int32_t>(s); }

/** \brief 从字符串到 uint32_t 的防溢出转换。
 * \throws 若转换值超出 int32_t 范围则抛出 std::out_of_range
 * \throws 若无法完成转换则抛出 std::invalid_argument
 */
template <typename T>
T stoui_generic(const std::string & s)
{
  static_assert(
    std::is_integral_v<T> && std::is_unsigned_v<T>, "T must be an unsigned integral type");

  size_t pos;
  const auto v = std::stoul(s, &pos);  // stoul returns unsigned long
  if (pos != s.length())
  {
    throw std::invalid_argument("Invalid characters in string");
  }
  if (v > std::numeric_limits<T>::max())
  {
    throw std::out_of_range("value not in range of target type");
  }

  return static_cast<T>(v);
}

// 常用无符号整数类型的显式实例化
inline uint8_t stoui8(const std::string & s) { return stoui_generic<uint8_t>(s); }
inline uint16_t stoui16(const std::string & s) { return stoui_generic<uint16_t>(s); }
inline uint32_t stoui32(const std::string & s) { return stoui_generic<uint32_t>(s); }

/**
 * \brief 将字符串转为小写。
 * \param string 输入字符串。
 * \return 输入字符串的小写形式。
 */
std::string to_lower_case(const std::string & string);

/**
 * \brief 从字符串解析布尔值。
 * \param bool_string 输入字符串，可为 "true"、"false"（大小写不敏感）
 * \return 解析得到的布尔值。
 * \throws 若字符串不是有效的布尔表示则抛出 std::invalid_argument。
 */
bool parse_bool(const std::string & bool_string);

template <typename T>
std::vector<T> parse_array(const std::string & array_string)
{
  // 使用 regex 检查是否为扁平数组：以 [ 开头、] 结尾，无嵌套方括号
  const std::regex array_regex(R"(^\[\s*([^\[\]]*\s*(,\s*[^\[\]]+\s*)*)?\]$)");
  if (!std::regex_match(array_string, array_regex))
  {
    throw std::invalid_argument(
      "String must be a flat array: starts with '[' and ends with ']', no nested arrays");
  }

  // 使用 regex 匹配空数组或仅含空白的表达式
  const std::regex empty_or_spaces_regex(R"(^\[\s*\]$)");
  if (std::regex_match(array_string, empty_or_spaces_regex))
  {
    return {};  // 输入为 "[]" 时返回空数组
  }

  // 使用 regex 检测逗号分隔但元素间仅有空白或没有问题的情况，例如 "[,]" "[a,b,,c]"
  const std::regex comma_separated_regex(R"(^\[\s*([^,\s]+(\s*,\s*[^,\s]+)*)?\s*\]$)");
  if (!std::regex_match(array_string, comma_separated_regex))
  {
    throw std::invalid_argument(
      "String must be a flat array with comma-separated values and no spaces between them");
  }

  std::vector<T> result = {};
  if (array_string == "[]")
  {
    return result;  // 输入为 "[]" 时返回空数组
  }

  // 匹配逗号分隔的值（元素间可无空格或含空格），例如 "[a,b,c]"、"[a]" 或 "[a, b, c]"
  const std::regex value_regex(R"([^\s,\[\]]+)");
  auto begin = std::sregex_iterator(array_string.begin(), array_string.end(), value_regex);
  auto end = std::sregex_iterator();

  for (auto it = begin; it != end; ++it)
  {
    const std::string value_str = it->str();  // 获取第一个捕获组
    if constexpr (std::is_same_v<T, std::string>)
    {
      result.push_back(value_str);
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
      result.push_back(parse_bool(value_str));
    }
    else if constexpr (std::is_floating_point_v<T> || std::is_integral_v<T>)
    {
      try
      {
        const T value = static_cast<T>(hardware_interface::stod(value_str));
        result.push_back(value);
      }
      catch (const std::exception &)
      {
        throw std::invalid_argument(
          "Failed converting string to floating point or integer: " + value_str);
      }
    }
    else
    {
      throw std::invalid_argument("Unsupported type for parsing: " + std::string(typeid(T).name()));
    }
  }
  return result;
}

std::vector<std::string> parse_string_array(const std::string & string_array_string);

}  // namespace hardware_interface

#endif  // HARDWARE_INTERFACE__LEXICAL_CASTS_HPP_