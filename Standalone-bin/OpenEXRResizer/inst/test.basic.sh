#!/usr/bin/env bash
export PATH=\
"/mnt/work/software/develop/dependences/open_exr/bin"\
:${PATH}

export LD_LIBRARY_PATH=\
"/mnt/work/software/develop/dependences/open_exr/lib"\
:${LD_LIBRARY_PATH}

ROOT_PATH=$(dirname "$(readlink -e "$BASH_SOURCE")")

${ROOT_PATH}/out/OpenEXRResizer -r 4 ${ROOT_PATH}/tst/test_nocrop.exr ${ROOT_PATH}/tst/beauty/proxy.test_nocrop.exr
${ROOT_PATH}/out/OpenEXRResizer -r 4 ${ROOT_PATH}/tst/test_croped.exr ${ROOT_PATH}/tst/beauty/proxy.test_croped.exr