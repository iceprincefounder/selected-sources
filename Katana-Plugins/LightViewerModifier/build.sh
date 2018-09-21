
source "/opt/rh/devtoolset-2/enable"

BUILD_DIR=/home/xukai/Git/build_repo/LightViewerModifier3.0
INSTALL_DIR=/home/xukai/Git/git_repo/katana/resource/3.0
KATANA_HOME=/mnt/work/software/katana/katana3.0v1b4
SRC_DIR=$(dirname "$(readlink -e "$BASH_SOURCE")")
echo ${SRC_DIR}
if [ ! -d ${BUILD_DIR} ]; then
      mkdir ${BUILD_DIR}
fi
cd ${BUILD_DIR}
cmake ${SRC_DIR} \
        -G "Unix Makefiles" \
        -DKatana_SDK_ROOT="/mnt/work/software/katana/katana3.0v1b4" \
        -DKatana_DIR="/mnt/work/software/katana/katana3.0v1b4/plugin_apis/cmake" \
        -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \

make install -j16