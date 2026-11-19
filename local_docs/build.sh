#!/usr/bin/env bash
# 把仓库里的 .rst 编成本地 HTML。改完文档后重新运行即可。
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VENV="$ROOT/local_docs/.venv"
OUT="$ROOT/local_docs/_build/html"

if [[ ! -x "$VENV/bin/sphinx-build" ]]; then
  python3 -m venv "$VENV"
  "$VENV/bin/pip" install -q -U pip
  "$VENV/bin/pip" install -q 'sphinx>=7' sphinx-rtd-theme sphinx-autobuild
fi

"$VENV/bin/sphinx-build" -b html -c "$ROOT/local_docs" "$ROOT" "$OUT"

echo
echo "文档已生成。用浏览器打开："
echo "  file://$OUT/index.html"
echo "硬件文档："
echo "  file://$OUT/hardware_interface/doc/hardware_components_userdoc.html"
