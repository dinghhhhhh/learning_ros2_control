:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/doc/semantic_components.rst

.. _semantic_components:

Semantic Components
---------------------------------------------------------

为简化常用硬件接口的配置，可使用所谓的 semantic components，对 claim / release interfaces 的机制进行封装。基类 ``semantic_components::SemanticComponentInterface`` 与 ``semantic_components::SemanticComponentCommandInterface`` 分别用于定义只读（read-only）与只写（write-only）设备的 semantic components。

现有 ``SemanticComponentInterface`` `(头文件链接) <https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/controller_interface/include/semantic_components/semantic_component_interface.hpp>`__ 及其关联的 broadcaster（如有）列表：

   * `IMUSensor <https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/controller_interface/include/semantic_components/imu_sensor.hpp>`__，由 :ref:`IMU Sensor Broadcaster <imu_sensor_broadcaster_userdoc>` 使用
   * `ForceTorqueSensor <https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/controller_interface/include/semantic_components/force_torque_sensor.hpp>`__，由 :ref:`Force Torque Sensor Broadcaster <force_torque_sensor_broadcaster_userdoc>` 使用
   * `GPSSensor <https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/controller_interface/include/semantic_components/gps_sensor.hpp>`__
   * `MagneticFieldSensor <https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/controller_interface/include/semantic_components/magnetic_field_sensor.hpp>`__
   * `PoseSensor <https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/controller_interface/include/semantic_components/pose_sensor.hpp>`__，由 :ref:`Pose Broadcaster <pose_broadcaster_userdoc>` 使用
   * `RangeSensor <https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/controller_interface/include/semantic_components/range_sensor.hpp>`__，由 :ref:`Range Sensor Broadcaster <range_sensor_broadcaster_userdoc>` 使用

现有 ``SemanticComponentCommandInterface`` `(头文件链接) <https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/controller_interface/include/semantic_components/semantic_component_command_interface.hpp>`__ 及其关联的 controller（如有）列表：

   * `LedRgbDevice <https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/controller_interface/include/semantic_components/led_rgb_device.hpp>`__
