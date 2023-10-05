#!/bin/bash

SCRIPT_FODLER=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
cd ..
PROJ_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
cd -

bash ${SCRIPT_FODLER}/build.sh ${PROJ_DIR} || exit 1

python3 -m venv ${PROJ_DIR}/.venv

source ${BASE_DIR}/.venv/bin/activate

export IMAGE_NAME=web_server

pytest --junitxml=${PROJ_DIR}/web_server.xml ${SCRIPT_FODLER}/web_server_tests.py

rm -rf ${PROJ_DIR}/.venv
