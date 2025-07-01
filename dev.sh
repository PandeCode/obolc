#!/usr/bin/env bash

set -euo pipefail

WATCH_DIR="${1:-.}"

inotifywait -r -m -e modify,create,delete,move "$WATCH_DIR" --format '%w%f' | while read -r file; do
  if [[ "$file" == *.cpp || "$file" == *.hpp ]]; then
    echo "🔄 Change detected: $file"
    pkill -9 obolc 2>/dev/null || true
    echo "🚀 Running make..."
    make run
  fi
done
