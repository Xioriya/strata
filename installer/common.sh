set -euo pipefail

TARGET_MNT="${TARGET_MNT:-/mnt/strata}"
STAGE3_BASE="${STRATA_STAGE3_BASE:-https://distfiles.gentoo.org/releases/amd64/autobuilds/current-stage3-amd64-openrc}"

die() {
  printf 'strata: %s\n' "$*" >&2
  exit 1
}

require_root() {
  [ "$(id -u)" -eq 0 ] || die "run as root"
}

require_tool() {
  command -v "$1" >/dev/null 2>&1 || die "missing tool: $1"
}

msg() {
  whiptail --title "$1" --msgbox "$2" 16 78
}

ask() {
  whiptail --title "$1" --yesno "$2" 16 78
}

input() {
  whiptail --title "$1" --inputbox "$2" 10 78 "${3:-}" 3>&1 1>&2 2>&3
}

menu() {
  whiptail --title "$1" --menu "$2" 24 80 14 "${@:3}" 3>&1 1>&2 2>&3
}

part_path() {
  local disk="$1"
  local number="$2"
  if [[ "$disk" =~ [0-9]$ ]]; then
    printf '%sp%s' "$disk" "$number"
  else
    printf '%s%s' "$disk" "$number"
  fi
}

list_disks() {
  lsblk -dpno NAME,SIZE,MODEL | grep -Ev 'loop|rom|zram|sr[0-9]' || true
}

select_partition() {
  local title="$1"
  local entries=()
  local name size fstype label type
  while read -r name size fstype label type; do
    [ "$type" = "part" ] || continue
    entries+=("$name" "$size ${fstype:-?} ${label:-}")
  done < <(lsblk -pno NAME,SIZE,FSTYPE,LABEL,TYPE)
  [ "${#entries[@]}" -gt 0 ] || die "no partitions found"
  menu "$title" "Select a partition." "${entries[@]}"
}
