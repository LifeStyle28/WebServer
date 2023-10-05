#!/bin/bash

PROJ_DIR=${1}

docker build -t web_server ${PROJ_DIR}
