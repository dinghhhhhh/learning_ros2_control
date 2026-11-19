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

/// \author Sai Kishor Kothakota

#ifndef HARDWARE_INTERFACE__TYPES__STATISTICS_TYPES_HPP_
#define HARDWARE_INTERFACE__TYPES__STATISTICS_TYPES_HPP_

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>

#include "hardware_interface/introspection.hpp"
#include "libstatistics_collector/moving_average_statistics/moving_average.hpp"
#include "libstatistics_collector/moving_average_statistics/types.hpp"
#if !defined(_WIN32) && !defined(__APPLE__)
#include "realtime_tools/mutex.hpp"
#define DEFAULT_MUTEX realtime_tools::prio_inherit_mutex
#else
#define DEFAULT_MUTEX std::mutex
#endif

namespace ros2_control
{
/**
 *  用于计算移动平均统计量的类。以常数内存、常数时间运行。
 * 注意：必须手动调用 reset() 才能开始新的测量窗口。
 *
 *  计算的统计量包括 average、maximum、minimum 以及 standard deviation（population）。
 *  均在在线方式下计算，无需存储观测数据。具体而言，average 为运行求和，
 * variance 通过 Welford 在线算法（参考：
 * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford%27s_online_algorithm）
 *  得到 standard deviation。
 *
 *  当统计量不可用时（例如尚未进行任何观测），返回 NaN。
 */
class MovingAverageStatistics
{
public:
  using StatisticData = libstatistics_collector::moving_average_statistics::StatisticData;
  MovingAverageStatistics() = default;

  ~MovingAverageStatistics() = default;

  /**
   *  返回已记录全部数据的算术平均值。若无观测，返回 NaN。
   *
   *  @return 已记录数据的算术平均值；若 sample count 为 0 则返回 NaN。
   */
  double get_average() const
  {
    std::lock_guard<DEFAULT_MUTEX> lock(mutex_);
    return statistics_data_.average;
  }

  /**
   *  返回已记录的最大值。若列表长度为 0，返回 NaN。
   *
   *  @return 已记录的最大值；若数据长度为 0 则返回 NaN。
   */
  double get_max() const
  {
    std::lock_guard<DEFAULT_MUTEX> lock(mutex_);
    return statistics_data_.max;
  }

  /**
   *  返回已记录的最小值。若列表长度为 0，返回 NaN。
   *
   *  @return 已记录的最小值；若数据长度为 0 则返回 NaN。
   */
  double get_min() const
  {
    std::lock_guard<DEFAULT_MUTEX> lock(mutex_);
    return statistics_data_.min;
  }

  /**
   *  返回已记录全部数据的 standard deviation（population）。若列表长度为 0，
   * 返回 NaN。
   *
   *  variance 通过 Welford 在线算法得到，参见
   * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford%27s_online_algorithm
   *
   *  @return 已记录数据的 standard deviation（population）；若数据长度为 0 则返回 NaN。
   */
  double get_standard_deviation() const
  {
    std::lock_guard<DEFAULT_MUTEX> lock(mutex_);
    return statistics_data_.standard_deviation;
  }

  /**
   *  返回包含 average、minimum、maximum、standard deviation（population）
   *  以及 sample count 的 StatisticData 对象。若无观测，
   * average、min、max 与 standard deviation 均为 NaN。
   *
   *  @return 包含 average、minimum、maximum、standard deviation（population）
   *  以及 sample count 的 StatisticData 对象。
   */
  const StatisticData & get_statistics_const_ptr() const
  {
    std::lock_guard<DEFAULT_MUTEX> lock(mutex_);
    return statistics_data_;
  }

  StatisticData get_statistics() const
  {
    std::lock_guard<DEFAULT_MUTEX> lock(mutex_);
    return statistics_data_;
  }

  /**
   *  获取当前测量值。
   *  即加入统计采集器的最后一个值。
   *
   *  @return 当前测量值；若尚未测量则返回 NaN。
   */
  const double & get_current_measurement_const_ptr() const
  {
    std::lock_guard<DEFAULT_MUTEX> lock(mutex_);
    return current_measurement_;
  }

  double get_current_measurement() const
  {
    std::lock_guard<DEFAULT_MUTEX> lock(mutex_);
    return current_measurement_;
  }

  /**
   *  重置全部计算值。相当于移动平均的新窗口。
   */
  void reset()
  {
    std::lock_guard<DEFAULT_MUTEX> lock(mutex_);
    statistics_data_.average = 0.0;
    statistics_data_.min = std::numeric_limits<double>::max();
    statistics_data_.max = std::numeric_limits<double>::lowest();
    statistics_data_.standard_deviation = 0.0;
    statistics_data_.sample_count = 0;
    current_measurement_ = std::numeric_limits<double>::quiet_NaN();
    sum_of_square_diff_from_mean_ = 0;
  }

  void reset_current_measurement()
  {
    std::lock_guard<DEFAULT_MUTEX> lock(mutex_);
    current_measurement_ = 0.0;
  }

  /**
   *  为当前窗口记录一个样本。输入值用于计算统计量。
   *  注意：任意 NaN 输入将被丢弃，不会作为测量加入。
   *
   *  @param item 观测到的值
   */
  void add_measurement(const double item)
  {
    std::lock_guard<DEFAULT_MUTEX> guard{mutex_};

    current_measurement_ = item;
    if (std::isfinite(item))
    {
      statistics_data_.sample_count = statistics_data_.sample_count + 1;
      const double previous_average = statistics_data_.average;
      statistics_data_.average =
        previous_average + (current_measurement_ - previous_average) /
                             static_cast<double>(statistics_data_.sample_count);
      statistics_data_.min = std::min(statistics_data_.min, current_measurement_);
      statistics_data_.max = std::max(statistics_data_.max, current_measurement_);
      sum_of_square_diff_from_mean_ =
        sum_of_square_diff_from_mean_ + (current_measurement_ - previous_average) *
                                          (current_measurement_ - statistics_data_.average);
      statistics_data_.standard_deviation = std::sqrt(
        sum_of_square_diff_from_mean_ / static_cast<double>(statistics_data_.sample_count));
    }
  }

  /**
   * 返回已观测的样本数量
   *
   * @return 已观测的样本数量
   */
  uint64_t get_count() const
  {
    std::lock_guard<DEFAULT_MUTEX> lock(mutex_);
    return statistics_data_.sample_count;
  }

private:
  mutable DEFAULT_MUTEX mutex_;
  StatisticData statistics_data_;
  double current_measurement_ = std::numeric_limits<double>::quiet_NaN();
  double sum_of_square_diff_from_mean_ = 0.0;
};

/**
 * @brief 用于保存移动平均统计量的数据结构。数据由 mutex 保护，可更新与读取。
 */
class MovingAverageStatisticsData
{
  using StatisticData = libstatistics_collector::moving_average_statistics::StatisticData;

public:
  MovingAverageStatisticsData()
  {
    reset();
    reset_statistics_sample_count_ = std::numeric_limits<unsigned int>::max();
  }

  /**
   * @brief 用新的统计量数据更新当前统计量数据。
   * @param statistics 用于更新当前统计量数据的统计采集器。
   */
  void update_statistics(const std::shared_ptr<MovingAverageStatistics> & statistics)
  {
    std::unique_lock<DEFAULT_MUTEX> lock(mutex_);
    if (statistics->get_count() > 0)
    {
      statistics_data_.average = statistics->get_average();
      statistics_data_.min = statistics->get_min();
      statistics_data_.max = statistics->get_max();
      statistics_data_.standard_deviation = statistics->get_standard_deviation();
      statistics_data_.sample_count = statistics->get_count();
      statistics_data_ = statistics->get_statistics();
      current_data_ = statistics->get_current_measurement();
    }
    if (statistics->get_count() >= reset_statistics_sample_count_)
    {
      statistics->reset();
    }
  }

  /**
   * @brief 设置重置统计量所需的样本数。
   * @param reset_sample_count 触发重置统计量的样本数。
   */
  void set_reset_statistics_sample_count(unsigned int reset_sample_count)
  {
    std::unique_lock<DEFAULT_MUTEX> lock(mutex_);
    reset_statistics_sample_count_ = reset_sample_count;
  }

  void reset()
  {
    statistics_data_.average = std::numeric_limits<double>::quiet_NaN();
    statistics_data_.min = std::numeric_limits<double>::quiet_NaN();
    statistics_data_.max = std::numeric_limits<double>::quiet_NaN();
    statistics_data_.standard_deviation = std::numeric_limits<double>::quiet_NaN();
    statistics_data_.sample_count = 0;
  }

  /**
   * @brief 获取统计量数据。
   * @return 统计量数据。
   */
  const StatisticData & get_statistics() const
  {
    std::unique_lock<DEFAULT_MUTEX> lock(mutex_);
    return statistics_data_;
  }

  const double & get_current_data() const
  {
    std::unique_lock<DEFAULT_MUTEX> lock(mutex_);
    return current_data_;
  }

private:
  /// 用于保护统计量数据的 mutex
  mutable DEFAULT_MUTEX mutex_;
  /// 统计量数据
  StatisticData statistics_data_;
  /// 当前数据值，用于计算统计量
  double current_data_ = std::numeric_limits<double>::quiet_NaN();
  /// 重置统计量所需的样本数
  unsigned int reset_statistics_sample_count_ = std::numeric_limits<unsigned int>::max();
};
}  // namespace ros2_control

namespace hardware_interface
{
/**
 * @brief 包含两个移动平均统计采集器的数据结构：一个用于 execution time，
 * 另一个用于 periodicity。
 */
struct HardwareComponentStatisticsCollector
{
  HardwareComponentStatisticsCollector()
  {
    execution_time = std::make_shared<ros2_control::MovingAverageStatistics>();
    periodicity = std::make_shared<ros2_control::MovingAverageStatistics>();
  }

  /**
   * @brief 重置 execution time 与 periodicity 统计采集器的内部统计量。
   */
  void reset_statistics()
  {
    execution_time->reset();
    periodicity->reset();
  }

  /// execution time 统计采集器
  std::shared_ptr<ros2_control::MovingAverageStatistics> execution_time = nullptr;
  /// periodicity 统计采集器
  std::shared_ptr<ros2_control::MovingAverageStatistics> periodicity = nullptr;
};
}  // namespace hardware_interface

#endif  // HARDWARE_INTERFACE__TYPES__STATISTICS_TYPES_HPP_