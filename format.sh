#!/usr/bin/env bash
#
# Formats all C++ source files with clang-format, using the same file
# selection as the "Check Formatting" step in the Build and Test workflow.
#
# Usage:
#   ./format.sh          # reformat files in place
#   ./format.sh --check  # only report files that are not formatted (as CI does)
#
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

if ! command -v clang-format >/dev/null 2>&1; then
    echo "error: clang-format not found in PATH" >&2
    exit 1
fi

mode=(-i)
if [[ "${1:-}" == "--check" ]]; then
    mode=(--dry-run --Werror)
fi

# Only search the source folders that exist
dirs=()
for dir in src tests; do
    [[ -d "$dir" ]] && dirs+=("$dir")
done

find "${dirs[@]}" -regex '.*\.\(h\|cpp\)' -print0 \
    | xargs -0 clang-format "${mode[@]}"
