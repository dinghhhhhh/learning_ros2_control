# 本地阅读 ros2_control 文档

源文件仍是仓库里的 `.rst`（例如 `hardware_interface/doc/`）。这里只负责把它们编成 HTML，不依赖 control.ros.org。

## 生成一次

```bash
./local_docs/build.sh
```

浏览器打开 `local_docs/_build/html/index.html`。硬件文档入口是 `hardware_interface/doc/hardware_components_userdoc.html`。改完 `.rst` 后重新运行该脚本。首次运行会在 `local_docs/.venv` 里创建虚拟环境并安装 Sphinx（该目录不需要提交）。

## 边改边看

```bash
./local_docs/preview.sh
```

浏览器打开 http://127.0.0.1:8765/ 。保存 `.rst` 后会自动重新构建并刷新。

## 和线上文档的差别

- 指向本仓库以外的 `:ref:`（例如 ros2_controllers、demos）会显示文字，但没有跳转。
- Controller Manager 参数表依赖 `generate_parameter_library`，本地只显示占位说明。
- `controller_manager/doc` 里引用的图片若未包含在本仓库中，页面上会缺图。
