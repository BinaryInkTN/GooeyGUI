#!/bin/bash

WAYLAND_PROTOCOLS_GIT="https://gitlab.freedesktop.org/wayland/wayland-protocols.git"
WLR_PROTOCOLS_GIT="https://gitlab.freedesktop.org/wlroots/wlr-protocols.git"
OUTPUTS=(xdg-shell xdg-dialog xdg-decorations wlr-data-control-unstable-v1)

SCRIPT_PATH=$0 
SCRIPT_PATH="$(realpath "$(dirname "${SCRIPT_PATH}")")"
SCRIPT_ROOT_PATH="${SCRIPT_PATH}/.."

echo "Script root path: $SCRIPT_ROOT_PATH"
OUTPUT_HEADER_DIR="${SCRIPT_ROOT_PATH}/internal/xdg"
OUTPUT_SRC_DIR="${SCRIPT_ROOT_PATH}/src/xdg"
WAYLAND_PROTOCOLS_DIR="${SCRIPT_PATH}/wayland-protocols"
WLR_PROTOCOLS_DIR="${SCRIPT_PATH}/wlr-protocols"


WAYLAND_PROTOCOLS=(
    "stable/xdg-shell/xdg-shell.xml"
    "unstable/xdg-dialog/xdg-dialog-unstable-v1.xml"
    "unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
)


if [[ -n "$(ls -A "${OUTPUT_HEADER_DIR}" 2>/dev/null)" ]]; then 
    echo "${OUTPUT_HEADER_DIR} is not empty, Nothing to do"
    echo "Exiting ..."
    exit 0 
fi

echo "Creating output directories..."
mkdir -p "${OUTPUT_SRC_DIR}"
mkdir -p "${OUTPUT_HEADER_DIR}"


echo "Cloning the Wayland Protocols repository..."
if [[ ! -d "$WAYLAND_PROTOCOLS_DIR" ]]; then
    git clone --depth 1 "$WAYLAND_PROTOCOLS_GIT" "${WAYLAND_PROTOCOLS_DIR}/" || {
        echo "Failed to clone wayland-protocols"
        exit 1
    }
else
    echo "Wayland protocols directory already exists, using existing clone."
fi


for i in "${!WAYLAND_PROTOCOLS[@]}"; do 
    protocol_path="${WAYLAND_PROTOCOLS_DIR}/${WAYLAND_PROTOCOLS[$i]}"
    
    if [[ ! -f "$protocol_path" ]]; then
        echo "Error: Protocol file not found: ${WAYLAND_PROTOCOLS[$i]}"
        continue
    fi

    echo "Processing: $protocol_path"

    
    header_out="${OUTPUT_HEADER_DIR}/${OUTPUTS[$i]}.h"
    src_out="${OUTPUT_SRC_DIR}/${OUTPUTS[$i]}.c"

    echo "Generating header: $header_out"
    if ! wayland-scanner client-header "$protocol_path" "$header_out"; then
        echo "Error generating header from $protocol_path"
        continue
    fi

    echo "Generating source: $src_out"
    if ! wayland-scanner private-code "$protocol_path" "$src_out"; then
        echo "Error generating source from $protocol_path"
        continue
    fi
done


echo "Cloning wlr-protocols repository..."
if [[ ! -d "$WLR_PROTOCOLS_DIR" ]]; then
    git clone --depth 1 "$WLR_PROTOCOLS_GIT" "${WLR_PROTOCOLS_DIR}/" || {
        echo "Failed to clone wlr-protocols"
        exit 1
    }
else
    echo "wlr-protocols directory already exists, using existing clone."
fi


WLR_DATA_CTL_XML=$(find "${WLR_PROTOCOLS_DIR}" -type f -name "*data-control*.xml" | head -n 1)

if [[ -z "$WLR_DATA_CTL_XML" ]]; then
    echo "Error: wlr-data-control protocol XML not found in ${WLR_PROTOCOLS_DIR}"
    exit 1
fi

echo "Found wlr-data-control protocol at: $WLR_DATA_CTL_XML"


header_out="${OUTPUT_HEADER_DIR}/${OUTPUTS[3]}.h"
src_out="${OUTPUT_SRC_DIR}/${OUTPUTS[3]}.c"

echo "Generating wlr-data-control header: $header_out"
if ! wayland-scanner client-header "$WLR_DATA_CTL_XML" "$header_out"; then
    echo "Error generating wlr-data-control header"
    exit 1
fi

echo "Generating wlr-data-control source: $src_out"
if ! wayland-scanner private-code "$WLR_DATA_CTL_XML" "$src_out"; then
    echo "Error generating wlr-data-control source"
    exit 1
fi


echo "Cleaning up temporary directories..."
rm -rf "${WAYLAND_PROTOCOLS_DIR}"
rm -rf "${WLR_PROTOCOLS_DIR}"

echo "All protocol files generated successfully:"
ls -l "${OUTPUT_HEADER_DIR}"
ls -l "${OUTPUT_SRC_DIR}"