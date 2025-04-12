#!/bin/bash
echo "------------------"
echo "RUNNING THE cJSON PATCH SCRIPT"
echo "------------------"

CURR_PATH="${PWD}"

ROOT_DIR="${CURR_PATH}"
CJSON_SUBMODULE_PATH="${ROOT_DIR}/third_party/cjson"
PATCH_FILE="${CURR_PATH}/scripts/cjson_submodule.diff" 
# echo "${ROOT_DIR}"
echo "${PATCH_FILE}"
# echo "${CJSON_SUBMODULE_FILE}"

pushd "${CJSON_SUBMODULE_PATH}" > /dev/null 2>&1

if git apply --check "${PATCH_FILE}" > /dev/null 2>&1; then
    git apply "${PATCH_FILE}" && echo "Patch applied to cJSON"
else
    echo "Patch already applied to cJSON"
fi
popd > /dev/null 2>&1
