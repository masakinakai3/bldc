#!/bin/bash
set -eu

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT_DIR"

echo "[phase6] Starting Phase6 run: $(date -u)"

echo "[phase6] make phase5"
if ! make phase5; then
    echo "[phase6] make phase5 FAILED" >&2
    exit 2
fi

echo "[phase6] make test_phase5"
if ! make test_phase5; then
    echo "[phase6] make test_phase5 FAILED" >&2
    exit 3
fi

echo "[phase6] Phase6 completed successfully: $(date -u)"
exit 0
