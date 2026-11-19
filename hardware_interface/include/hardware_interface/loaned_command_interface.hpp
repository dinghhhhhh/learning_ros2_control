// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#ifndef HARDWARE_INTERFACE__LOANED_COMMAND_INTERFACE_HPP_
#define HARDWARE_INTERFACE__LOANED_COMMAND_INTERFACE_HPP_

#include <functional>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include "hardware_interface/handle.hpp"
#include "rclcpp/logging.hpp"

namespace hardware_interface
{
class LoanedCommandInterface
{
public:
  using Deleter = std::function<void(void)>;

  explicit LoanedCommandInterface(CommandInterface::SharedPtr command_interface)
  : LoanedCommandInterface(command_interface, nullptr)
  {
  }

  explicit LoanedCommandInterface(CommandInterface::SharedPtr command_interface, Deleter && deleter)
  : command_interface_(command_interface),
    interface_name_(command_interface->get_name()),
    deleter_(std::forward<Deleter>(deleter))
  {
  }

  LoanedCommandInterface(const LoanedCommandInterface & other) = delete;

  LoanedCommandInterface(LoanedCommandInterface && other) = default;

  virtual ~LoanedCommandInterface()
  {
    if (command_interface_)
    {
      auto logger = rclcpp::get_logger(interface_name_);
      RCLCPP_WARN_EXPRESSION(
        logger,
        (get_value_statistics_.failed_counter > 0 || get_value_statistics_.timeout_counter > 0),
        "LoanedCommandInterface %s has %u (%.4f %%) timeouts and %u (~ %.4f %%) missed calls out "
        "of "
        "%u get_value calls",
        interface_name_.c_str(), get_value_statistics_.timeout_counter,
        (get_value_statistics_.timeout_counter * 100.0) / get_value_statistics_.total_counter,
        get_value_statistics_.failed_counter,
        (get_value_statistics_.failed_counter * 100.0) / get_value_statistics_.total_counter,
        get_value_statistics_.total_counter);
      RCLCPP_WARN_EXPRESSION(
        logger,
        (set_value_statistics_.failed_counter > 0 || set_value_statistics_.timeout_counter > 0),
        "LoanedCommandInterface %s has %u (%.4f %%) timeouts and  %u (~ %.4f %%) missed calls out "
        "of "
        "%u set_value calls",
        interface_name_.c_str(), set_value_statistics_.timeout_counter,
        (set_value_statistics_.timeout_counter * 100.0) / set_value_statistics_.total_counter,
        set_value_statistics_.failed_counter,
        (set_value_statistics_.failed_counter * 100.0) / set_value_statistics_.total_counter,
        set_value_statistics_.total_counter);
    }
    if (deleter_)
    {
      deleter_();
    }
  }

  const std::string & get_name() const { return command_interface_->get_name(); }

  const std::string & get_interface_name() const
  {
    return command_interface_->get_interface_name();
  }

  const std::string & get_prefix_name() const { return command_interface_->get_prefix_name(); }

  /**
   * @brief 设置 command interface 的值。
   * @tparam T 待设置值的类型。
   * @param value 要设置的值。
   * @param max_tries 设置值的最大尝试次数。
   * @return 成功设置为 true，否则为 false。
   *
   * @note 本方法线程安全且非阻塞。
   * @note 多线程同时访问内部 Handle 且无法加锁写入时返回 false；成功则更新并返回 true。
   * @note 失败前最多尝试 max_tries 次，尝试间会 yield 线程；一旦成功立即返回 true。
   */
  template <typename T>
  [[nodiscard]] bool set_value(const T & value, unsigned int max_tries = 10)
  {
    unsigned int nr_tries = 0;
    ++set_value_statistics_.total_counter;
    while (!command_interface_->set_limited_value(value))
    {
      ++set_value_statistics_.failed_counter;
      ++nr_tries;
      if (nr_tries == max_tries)
      {
        ++set_value_statistics_.timeout_counter;
        return false;
      }
      std::this_thread::yield();
    }
    return true;
  }

  /**
   * @brief 获取 command interface 的值。
   * @tparam T 待读取值的类型。
   * @return 成功访问时返回 command interface 的值，否则 std::nullopt。
   * @param max_tries 获取值的最大尝试次数。
   *
   * @note 本方法线程安全且非阻塞。
   * @note 多线程同时访问内部 Handle 且无法加锁读取时返回 std::nullopt；成功则返回值。
   * @note 失败前最多尝试 max_tries 次，尝试间会 yield 线程；一旦成功立即返回值。
   */
  template <typename T = double>
  [[nodiscard]] std::optional<T> get_optional(unsigned int max_tries = 10) const
  {
    unsigned int nr_tries = 0;
    do
    {
      ++get_value_statistics_.total_counter;
      const std::optional<T> data = command_interface_->get_optional<T>();
      if (data.has_value())
      {
        return data;
      }
      ++get_value_statistics_.failed_counter;
      ++nr_tries;
      std::this_thread::yield();
    } while (nr_tries < max_tries);

    ++get_value_statistics_.timeout_counter;
    return std::nullopt;
  }

  /**
   * @brief 获取 command interface 的数据类型。
   * @return command interface 的数据类型。
   */
  HandleDataType get_data_type() const { return command_interface_->get_data_type(); }

  /**
   * @brief 检查 command interface 是否可转换为 double。
   * @return 可转换则为 true，否则为 false。
   */
  bool is_castable_to_double() const { return command_interface_->is_castable_to_double(); }

protected:
  CommandInterface::SharedPtr command_interface_;
  std::string interface_name_;
  Deleter deleter_;

private:
  struct HandleRTStatistics
  {
    unsigned int total_counter = 0;
    unsigned int failed_counter = 0;
    unsigned int timeout_counter = 0;
  };
  mutable HandleRTStatistics get_value_statistics_;
  HandleRTStatistics set_value_statistics_;
};

}  // namespace hardware_interface
#endif  // HARDWARE_INTERFACE__LOANED_COMMAND_INTERFACE_HPP_
