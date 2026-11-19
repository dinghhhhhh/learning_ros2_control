# 本地预览用 Sphinx 配置。源文件仍是仓库里的 .rst，不改上游文档结构。
# 用法见 local_docs/README.md

from docutils import nodes
from docutils.parsers.rst import Directive

project = "ros2_control"
copyright = "ros2_control development team"
author = "ros2_control development team"
language = "zh_CN"
# 站点首页必须是源码根目录的 index.rst，生成物才会是 html/index.html。
# doc/index.rst 与此重复，且路径相对 doc/，不作为首页。
master_doc = "index"
source_suffix = ".rst"

exclude_patterns = [
    "local_docs/**",
    "**/.git/**",
    "**/CHANGELOG.rst",
    "**/build/**",
    "**/install/**",
    "**/log/**",
    # 这两处依赖 rosdoc2 生成的 API 页，本地用户文档用不到
    "ros2_control/**",
    "transmission_interface/**",
    "doc/index.rst",
]

html_theme = "sphinx_rtd_theme"
html_title = "ros2_control 本地文档"
html_show_sourcelink = False


class _SkipGeneratedDirective(Directive):
    """control.ros.org 上由 generate_parameter_library 展开，本地只保留占位。"""

    has_content = True
    required_arguments = 0
    optional_arguments = 0

    def run(self):
        text = nodes.paragraph(
            text="本地预览未展开此段。完整参数表在 control.ros.org 构建时由 generate_parameter_library 生成。"
        )
        return [nodes.admonition("", nodes.title(text="参数表"), text)]


def _missing_reference(app, env, node, contnode):
    # 指向 ros2_controllers、demos 等外部文档的 :ref: 在本仓库里没有目标。
    # 保留可见文字，避免整站构建失败。
    return contnode


def setup(app):
    app.add_directive("generate_parameter_library_details", _SkipGeneratedDirective)
    app.add_directive("generate_parameter_library_default", _SkipGeneratedDirective)
    app.connect("missing-reference", _missing_reference)
