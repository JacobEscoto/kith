#!/usr/bin/bash

set -euo pipefail

# Config
readonly PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly BUILD_DIR="${PROJECT_ROOT}/build"
readonly BIN_NAME="kith"
readonly INSTALL_PREFIX="/usr/local/bin"
readonly DESKTOP_SRC="${PROJECT_ROOT}/resources/kith.desktop"
readonly AUTOSTART_DIR="${HOME}/.config/autostart"

NONINTERACTIVE=0
UNINSTALL=0

for arg in "$@"; do
  case "$arg" in
  --yes | -y) NONINTERACTIVE=1 ;;
  --uninstall) UNINSTALL=1 ;;
  --help | -h)
    echo "Usage: $0 [flags]"
    echo "Flags:"
    echo "  -y, --yes      Non interactive flag. Uses Yes as default prompt confirmation"
    echo "  --uninstall    It removes the binary and autostart"
    echo "  -h, --help     Shows help documentation"
    exit 0
    ;;
  *)
    echo "Unknown argument: $arg" >&2
    exit 1
    ;;
  esac
done

# Prompts confirmation [Y/n]. Default is yes if pressing Enter.
confirm() {
  local prompt="$1"
  if [[ "$NONINTERACTIVE" -eq 1 ]]; then
    return 0
  fi
  local reply
  read -r -p "${prompt} [Y/n]: " reply
  reply="${reply:-Y}"
  [[ "$reply" =~ ^([yY])$ ]]
}

# Verifies if 'ninja' is in the system PATH
is_ninja_installed() {
  if command -v ninja &>/dev/null; then
    echo "true"
  else
    echo "false"
  fi
}

if [[ "$UNINSTALL" -eq 1 ]]; then
  echo ""
  echo "  - ${INSTALL_PREFIX}/${BIN_NAME}"
  echo "  - ${AUTOSTART_DIR}/kith.desktop"
  if confirm "Confirm deletion?"; then
    sudo rm -f "${INSTALL_PREFIX}/${BIN_NAME}"
    rm -f "${AUTOSTART_DIR}/kith.desktop"
    echo "[SUCCESS] Kith uninstalled."
  else
    echo "[ERROR] Cancelled."
  fi
  exit 0
fi

echo ">>> Checking dependencies..."
missing_pkgs=()

command -v cmake >/dev/null 2>&1 || missing_pkgs+=(cmake)
command -v g++ >/dev/null 2>&1 || missing_pkgs+=(build-essential)

if pkg-config --exists Qt6Widgets 2>/dev/null; then
  QT_FOUND="Qt6"
elif pkg-config --exists Qt5Widgets 2>/dev/null; then
  QT_FOUND="Qt5"
else
  QT_FOUND=""
  missing_pkgs+=(qt6-base-dev qt6-base-dev-tools)
fi

if [[ -n "$QT_FOUND" ]]; then
  echo "Found ${QT_FOUND}."
fi

if [[ "${#missing_pkgs[@]}" -gt 0 ]]; then
  echo "Missing packages: ${missing_pkgs[*]}"
  if ! command -v apt >/dev/null 2>&1; then
    echo "[ERROR] Could not found 'apt'. Install manually: ${missing_pkgs[*]}"
    exit 1
  fi
  if confirm "Install the missing packages with 'sudo apt install'?"; then
    sudo apt update
    sudo apt install -y "${missing_pkgs[@]}"
  else
    echo "[ERROR] Could not continue without the dependencies. Aborting..."
    exit 1
  fi
fi

# build
echo ">>> Compiling at ${BUILD_DIR}..."

cmake_gen_args=()
if [[ "$(is_ninja_installed)" == "true" ]]; then
  echo "Ninja detected, it will be used as the generator (-G Ninja)."
  cmake_gen_args=(-G Ninja)
else
  echo "[WARNING] Ninja was not detected, Make will be used as the default generator."
fi

cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" "${cmake_gen_args[@]}" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --parallel "$(nproc)"

BUILT_BIN="${BUILD_DIR}/${BIN_NAME}"
if [[ ! -x "$BUILT_BIN" ]]; then
  echo "[ERROR] Could not found the compiled binary at ${BUILT_BIN}."
  exit 1
fi
echo "[SUCCESS] Successful compilation."

echo
echo "The binary will be copied to: ${INSTALL_PREFIX}/${BIN_NAME}"
echo "This requires admin permissions (sudo)."
if confirm "Install at ${INSTALL_PREFIX}?"; then
  sudo install -Dm755 "$BUILT_BIN" "${INSTALL_PREFIX}/${BIN_NAME}"
  echo "[SUCCESS] Installed at ${INSTALL_PREFIX}/${BIN_NAME}"
  INSTALL_SYSTEMWIDE=1
else
  echo "Installation at ${INSTALL_PREFIX} skipped. The binary is available at: "
  echo "  ${BUILT_BIN}"
  INSTALL_SYSTEMWIDE=0
fi

if [[ "$INSTALL_SYSTEMWIDE" -eq 1 ]] && [[ -f "$DESKTOP_SRC" ]]; then
  echo
  if confirm "Do you want to add the widget at the session's autostart (~/.config/autostart)?"; then
    mkdir -p "$AUTOSTART_DIR"
    cp "$DESKTOP_SRC" "$AUTOSTART_DIR/"
    echo "[SUCCESS] Autostart configured."
  fi
fi

echo
echo "Ready. You can run it right now with:"
if [[ "$INSTALL_SYSTEMWIDE" -eq 1 ]]; then
  echo "  ${BIN_NAME}"
else
  echo "  ${BUILT_BIN}"
fi
