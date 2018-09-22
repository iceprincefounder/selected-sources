#!/usr/bin/env bash
export PATH=\
"/mnt/work/software/develop/dependences/open_exr/bin"\
:${PATH}

export LD_LIBRARY_PATH=\
"/mnt/work/software/develop/dependences/open_exr/lib"\
:${LD_LIBRARY_PATH}

ROOT_PATH=$(dirname "$(readlink -e "$BASH_SOURCE")")

TEST_DIR="/mnt/public/home/xukai/factory2.0/OpenEXRResizer/output"

${ROOT_PATH}/OpenEXRResizer/runResizer -r 2 -f $TEST_DIR -t 8