:github_url: https://github.com/ros-controls/ros2_control/blob/{REPOS_FILE_BRANCH}/hardware_interface/doc/joint_limiting.rst

.. _joint_limiting:


Joint Limiting for ros2_control
---------------------------------------------------------

ros2_control 提供多种机制，用于为不同的 hardware interfaces 处理 joint limits。

Enable Joint Limits
#####################

在 ros2_control 中，可通过以下几种方式控制 joint limits 的处理：

* 通过 ``ros2_control_node`` 参数 ``enforce_command_limits`` 对所有 hardware components 的全部 interfaces 全局生效，详见 :ref:`此处 <doc/ros2_control/controller_manager/doc/userdoc:parameters>`。
* 对某个 joint 的全部 interfaces

  .. code-block:: xml

      <joint name="joint1">
        ...
        <limits enable="false"/>
        ...
      </joint>
* 对某个 joint 的单个 interface

  .. code-block:: xml

      <joint name="joint1">
        ...
        <command_interface name="position">
          <limits enable="false"/>
        </command_interface>
        ...
      </joint>

若某个 interface 启用了 joint limits，controller_manager 在启动时会打印类似如下消息：

  .. code-block:: text

    [controller_manager]: Using JointLimiter for joint 'joint1' in hardware 'RRBot' : '  has position limits: true [-1, 1]
    [ros2_control_node-1]   has velocity limits: true [1]
    [ros2_control_node-1]   has acceleration limits: false [nan]
    [ros2_control_node-1]   has deceleration limits: false [nan]
    [ros2_control_node-1]   has jerk limits: false [nan]
    [ros2_control_node-1]   has effort limits: true [100]
    [ros2_control_node-1]   angle wraparound: true'

Configuration of Limits
########################
tba

Description of the Limiter Algorithms
######################################
tba
