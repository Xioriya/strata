#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/step/step"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

pass=0
fail=0

ok() { pass=$((pass + 1)); printf 'ok   %s\n' "$1"; }
no() { fail=$((fail + 1)); printf 'FAIL %s\n' "$1"; }

expect_status() {
  local want="$1"
  local name="$2"
  shift 2
  set +e
  "$@" >"$TMP/out" 2>"$TMP/err"
  local got=$?
  set -e
  if [[ "$got" == "$want" ]]; then
    ok "$name"
  else
    no "$name (status $got, want $want)"
    sed 's/^/     /' "$TMP/err"
  fi
}

expect_contains() {
  local needle="$1"
  local name="$2"
  if grep -Fq -- "$needle" "$TMP/out"; then
    ok "$name"
  else
    no "$name (missing: $needle)"
    sed 's/^/     /' "$TMP/out"
  fi
}

expect_err_contains() {
  local needle="$1"
  local name="$2"
  if grep -Fq -- "$needle" "$TMP/err"; then
    ok "$name"
  else
    no "$name (missing: $needle)"
    sed 's/^/     /' "$TMP/err"
  fi
}

export STRATA_CONFIG_ROOT="$TMP/etc/portage"
export STRATA_REPO_ROOT="$ROOT/tests/fixtures/repo"
export STRATA_DISTDIR="$TMP/distfiles"
export STRATA_VDB_ROOT="$TMP/pkg"

mkdir -p "$STRATA_CONFIG_ROOT/repos.conf" "$STRATA_DISTDIR" "$STRATA_VDB_ROOT/app-misc/installed-1.0"

cat >"$STRATA_CONFIG_ROOT/make.conf" <<'EOF'
COMMON_FLAGS="-O2 -pipe"
CFLAGS="${COMMON_FLAGS}"
MAKEOPTS="-j8"
PORTAGE_BINHOST="https://example.invalid/binpkgs"
EOF

cat >"$STRATA_CONFIG_ROOT/repos.conf/gentoo.conf" <<'EOF'
[gentoo]
location = /var/db/repos/gentoo
EOF

expect_status 0 "version" "$BIN" --version
expect_contains "step 0.0.1" "version text"

expect_status 0 "help" "$BIN" --help
expect_contains "unmask" "help lists unmask"
expect_contains "which" "help lists which"

expect_status 0 "info" "$BIN" --info
expect_contains "CFLAGS=-O2 -pipe" "info expands variables"
expect_contains "installed   : 1 packages" "info counts vdb"
expect_contains "binhost     : https://example.invalid/binpkgs" "info binhost"
expect_contains "repositories: gentoo /var/db/repos/gentoo" "info repositories"

expect_status 0 "search" "$BIN" --search neovim
expect_contains "app-editors/neovim" "search finds neovim"

expect_status 0 "which newest" "$BIN" which app-editors/neovim
expect_contains "app-editors/neovim/neovim-0.10.2-r1.ebuild" "which selects newest ebuild"

expect_status 0 "which exact" "$BIN" which app-editors/neovim-0.9.5
expect_contains "neovim-0.9.5.ebuild" "which selects exact version"

expect_status 1 "which missing version" "$BIN" which app-editors/neovim-9.9.9
expect_status 1 "which missing package" "$BIN" which app-misc/missing
expect_status 2 "which needs atom" "$BIN" which

expect_status 0 "unmask" "$BIN" unmask app-editors/neovim
expect_contains "app-editors/neovim ~amd64" "unmask appends keyword"
expect_status 0 "unmask again" "$BIN" unmask app-editors/neovim
if grep -Fq "no changes" "$TMP/out"; then
  ok "unmask deduplicates"
else
  no "unmask deduplicates"
fi
if [[ "$(wc -l <"$STRATA_CONFIG_ROOT/package.accept_keywords/strata.conf")" == "1" ]]; then
  ok "unmask writes one line"
else
  no "unmask writes one line"
fi

expect_status 0 "mask" "$BIN" mask sys-apps/foo
expect_contains "sys-apps/foo" "mask appends atom"

expect_status 0 "use" "$BIN" use app-editors/neovim wayland lua
expect_contains "app-editors/neovim wayland lua" "use appends flags"

expect_status 0 "sync pretend" "$BIN" --pretend --sync
expect_err_contains "git clone --depth=1 https://github.com/gentoo/gentoo" "sync prints clone"

expect_status 0 "no args" "$BIN"
expect_contains "Usage:" "no args prints help"

printf '\n%d passed, %d failed\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
