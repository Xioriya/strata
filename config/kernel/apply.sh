#!/usr/bin/env bash
set -euo pipefail

SRC="${1:-/usr/src/linux}"
FRAGMENT="${2:-$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/strata.fragment}"

[ -d "$SRC" ] || { printf 'kernel source not found: %s\n' "$SRC" >&2; exit 1; }
[ -f "$SRC/scripts/kconfig/merge_config.sh" ] || { printf 'merge_config.sh not found\n' >&2; exit 1; }
[ -f "$FRAGMENT" ] || { printf 'fragment not found: %s\n' "$FRAGMENT" >&2; exit 1; }

cd "$SRC"
./scripts/kconfig/merge_config.sh .config "$FRAGMENT"
make olddefconfig
