#!/usr/bin/env bash
ROOT_PATH=$(dirname "$(readlink -e "$BASH_SOURCE")")

TEST_DIR="/mnt/public/home/xukai/factory2.0/OpenEXRResizer/m90267"

${ROOT_PATH}/OpenEXRResizer/runResizer -r 2 -f $TEST_DIR -t 12