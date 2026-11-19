// Copyright 2025 PAL Robotics S.L.
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

#ifndef HARDWARE_INTERFACE__HELPERS_HPP_
#define HARDWARE_INTERFACE__HELPERS_HPP_

#include <algorithm>
#include <map>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace ros2_control
{

/**
 * @brief 获取容器中某元素对应的迭代器。
 * @param container 待搜索的容器。
 * @param item 待查找的元素。
 * @return 指向容器中该元素的迭代器。
 *
 * @note 仅支持 std::vector、std::map 与 std::unordered_map。
 */
template <typename Container, typename T>
[[nodiscard]] auto get_item_iterator(const Container & container, const T & item)
{
  if constexpr (std::is_same_v<Container, std::vector<T>>)
  {
    return std::find(container.cbegin(), container.cend(), item);
  }
  else if constexpr (
    std::is_same_v<Container, std::map<T, typename Container::mapped_type>> ||
    std::is_same_v<Container, std::unordered_map<T, typename Container::mapped_type>>)
  {
    return container.find(item);
  }
  else
  {
    using is_vector = std::is_same<Container, std::vector<T>>;
    using is_map = std::is_same<Container, std::map<T, typename Container::mapped_type>>;
    using is_unordered_map =
      std::is_same<Container, std::unordered_map<T, typename Container::mapped_type>>;
    // 不支持的容器类型
    static_assert(
      is_vector::value || is_map::value || is_unordered_map::value,
      "Only std::vector, std::map and std::unordered_map are supported.");
  }
}

/**
 * @brief 检查元素是否在容器中。
 * @param container 待搜索的容器。
 * @param item 待查找的元素。
 * @return 若在容器中则为 true，否则为 false。
 */
template <typename Container, typename T>
[[nodiscard]] bool has_item(const Container & container, const T & item)
{
  return get_item_iterator(container, item) != container.cend();
}

/**
 * @brief 若元素尚不存在，则将其加入容器。
 * @param vector 待添加元素的容器。
 * @param item 待添加的元素。
 */
template <typename T>
void add_item(std::vector<T> & vector, const T & item)
{
  if (!has_item(vector, item))
  {
    vector.push_back(item);
  }
}

/**
 * @brief 将另一容器中的元素加入本容器（若尚不存在）。
 * @param vector 待添加元素的容器。
 * @param items 提供待添加元素的容器。
 */
template <typename T>
void add_items(std::vector<T> & vector, const std::vector<T> & items)
{
  for (const auto & item : items)
  {
    add_item(vector, item);
  }
}

/**
 * @brief 若元素在容器中，则将其移除。
 * @param container 待移除元素的容器。
 * @param item 待移除的元素。
 * @return 若已移除则为 true，否则为 false。若元素不在容器中则返回 false。
 */
template <typename Container>
[[nodiscard]] bool remove_item(Container & container, typename Container::const_reference item)
{
  auto it = get_item_iterator(container, item);
  if (it != container.end())
  {
    container.erase(it);
    return true;
  }
  return false;
}

/**
 * @brief 检查容器是否包含待查元素中的任意一个。
 * @param container 待搜索的容器。
 * @param items 待查找的元素列表。
 * @return 若包含其中任意元素则为 true，否则为 false。
 */
template <typename Container>
[[nodiscard]] bool has_any_item(
  const Container & container, const std::vector<typename Container::key_type> & items)
{
  return std::any_of(
    items.begin(), items.end(),
    [&container](const typename Container::key_type & item) { return has_item(container, item); });
}

/**
 * @brief 检查容器是否包含待查元素中的任意一个。
 * @param container 待搜索的容器。
 * @param items 待查找的元素列表。
 * @return 若包含其中任意元素则为 true，否则为 false。
 */
template <typename T>
[[nodiscard]] bool has_any_item(const std::vector<T> & container, const std::vector<T> & items)
{
  return std::any_of(
    items.begin(), items.end(), [&container](const T & item) { return has_item(container, item); });
}

/**
 * @brief 检查容器是否包含待查元素中的全部元素。
 * @param container 待搜索的容器。
 * @param items 待查找的元素列表。
 * @return 若包含全部元素则为 true，否则为 false。
 */
template <typename Container>
[[nodiscard]] bool has_all_items(
  const Container & container, const std::vector<typename Container::key_type> & items)
{
  return std::all_of(
    items.begin(), items.end(),
    [&container](const typename Container::key_type & item) { return has_item(container, item); });
}

/**
 * @brief 检查容器是否包含待查元素中的全部元素。
 * @param container 待搜索的容器。
 * @param items 待查找的元素列表。
 * @return 若包含全部元素则为 true，否则为 false。
 */
template <typename T>
[[nodiscard]] bool has_all_items(const std::vector<T> & container, const std::vector<T> & items)
{
  return std::all_of(
    items.begin(), items.end(), [&container](const T & item) { return has_item(container, item); });
}

/**
 * @brief 检查容器中元素是否全部唯一。
 * @param container 待搜索的容器。
 * @return 若全部唯一则为 true，否则为 false。
 *
 * @note 容器必须可排序。
 * @note 容器必须具有 begin() 与 end() 方法。
 */
template <typename Container>
[[nodiscard]] bool is_unique(Container container)
{
  std::sort(container.begin(), container.end());
  return std::adjacent_find(container.cbegin(), container.cend()) == container.cend();
}

/**
 * @brief 去除字符串首尾空白字符。
 * @param str 待处理的字符串。
 * @return 去除空白后的字符串。
 */
inline std::string strip(const std::string & str)
{
  const std::string whitespace = " \t\n\r\f\v";
  size_t start = str.find_first_not_of(whitespace);
  if (start == std::string::npos)
  {
    return "";  // 字符串全部为空白
  }
  size_t end = str.find_last_not_of(whitespace);
  return str.substr(start, end - start + 1);
}

}  // namespace ros2_control

#endif  // HARDWARE_INTERFACE__HELPERS_HPP_