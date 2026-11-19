# ros2_control

这是 [ros-controls/ros2_control](https://github.com/ros-controls/ros2_control) 的学习用副本。文档和 `hardware_interface` 的注释已译成中文，专业术语保持英文。许可证仍为 [Apache 2.0](LICENSE)。

本仓库是 ros2_control 的**核心框架**，不是整套控制栈。具体控制器（如差速、关节轨迹）在 [ros2_controllers](https://github.com/ros-controls/ros2_controllers)，示例在 [ros2_control_demos](https://github.com/ros-controls/ros2_control_demos)。

## 它做什么

`controller_manager` 是运行入口。每个控制周期按这个顺序执行：

1. **read**：各 Hardware Component 从硬件读取状态，写入 state interface。
2. **update**：已激活的 controller 读取 state interface，算出指令，写入 command interface。
3. **write**：Hardware Component 从 command interface 取出指令，发给硬件。

硬件在 URDF 的 `<ros2_control>` 标签里声明，由 `pluginlib` 加载。类型有三种：

- **System**：多关节系统，例如整台机器人或移动底盘。
- **Actuator**：单个执行器。
- **Sensor**：只提供状态、不接受运动指令。

控制器和硬件都走 ROS 2 Lifecycle：`UNCONFIGURED` → `INACTIVE` → `ACTIVE`。

## 包

| 包 | 作用 |
|---|---|
| `hardware_interface` | 硬件抽象。`SystemInterface`、`ActuatorInterface`、`SensorInterface`，以及 `ResourceManager` |
| `controller_interface` | 控制器基类，以及 IMU、力传感器等 semantic component |
| `controller_manager` | 控制循环、控制器加载与切换、硬件资源管理 |
| `controller_manager_msgs` | Controller Manager 的消息与服务 |
| `joint_limits` | 关节限位。控制器或硬件都可以使用 |
| `transmission_interface` | 在 actuator 空间和 joint 空间之间映射位置、速度、力矩 |
| `ros2controlcli` | `ros2 control` 命令行 |
| `rqt_controller_manager` | Controller Manager 的图形界面 |
| `hardware_interface_testing` | 硬件插件测试夹具 |
| `ros2_control_test_assets` | 测试用的 URDF 等资源 |
| `ros2_control` | 把以上包收在一起的 metapackage |

## 本地阅读中文文档

文档源文件是各目录下的 `.rst`。生成 HTML：

```bash
./local_docs/build.sh
```

用浏览器打开 `local_docs/_build/html/index.html`。改完 `.rst` 后重新运行该脚本；若要保存后自动刷新，运行 `./local_docs/preview.sh`，再打开 http://127.0.0.1:8765/ 。

## 和上下游的关系

- 编译本仓库还依赖 `realtime_tools`、`control_msgs` 等，版本清单原在 `ros2_control.<发行版>.repos`。
- `ros2_controllers`、`gz_ros2_control`、演示程序等不属于本仓库，原清单在 `ros_controls.<发行版>.repos`。
