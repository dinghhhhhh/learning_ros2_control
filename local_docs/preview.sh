#!/usr/bin/env bash
# 启动本地网页，并在 .rst 保存后自动重新构建。
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VENV="$ROOT/local_docs/.venv"
OUT="$ROOT/local_docs/_build/html"
PORT="${PORT:-8765}"

if [[ ! -x "$VENV/bin/sphinx-autobuild" ]]; then
  python3 -m venv "$VENV"
  "$VENV/bin/pip" install -q -U pip
  "$VENV/bin/pip" install -q 'sphinx>=7' sphinx-rtd-theme sphinx-autobuild
fi

echo "浏览器打开 http://127.0.0.1:${PORT}/"
echo "修改并保存 .rst 后页面会自动刷新。Ctrl+C 结束。"

exec "$VENV/bin/sphinx-autobuild" \
  -b html \
  -c "$ROOT/local_docs" \
  "$ROOT" "$OUT" \
  --host 127.0.0.1 \
  --port "$PORT" \
  --watch "$ROOT/doc" \
  --watch "$ROOT/hardware_interface/doc" \
  --watch "$ROOT/controller_manager/doc" \
  --watch "$ROOT/ros2controlcli/doc"
