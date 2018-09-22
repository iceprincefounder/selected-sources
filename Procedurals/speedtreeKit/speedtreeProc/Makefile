
INSTALL_DIR = ../Release

OUT_PATH = out
SRC_PATH = source
ARNOLD_PATH = /mnt/work/software/arnold/arnold-5.0.1.0
ST_LIB_PATH = ../Release
SPEEDTREE_SDK_PATH = /mnt/work/software/lib/Speedtree-SDK7.0

LOGFILE=$(LOGPATH)`date +'%H:%M:%S'`
CXX = c++
CXXFLAGS = -O2 -fPIC -shared -Wall

INCLUDES = -I${ARNOLD_PATH}/include
INCLUDES += -I${ST_LIB_PATH}/include
INCLUDES += -I${SPEEDTREE_SDK_PATH}/Include
LIBS = -L${ARNOLD_PATH}/bin -lai
LIBS += -L${ST_LIB_PATH}/lib -lSpeedtreeReader
LIBS += -L${SPEEDTREE_SDK_PATH}/lib -lSpeedTreeCore_fPIC

.PHONY: clean default install start

all:start procedural shader

procedural:
	@echo [${LOGFILE}] "--Start Compilie Procedural..."
	@${CXX} ${CXXFLAGS} ${SRC_PATH}/speedtree_procedural.cpp -o ${OUT_PATH}/speedtree_procedural.so ${INCLUDES} ${LIBS}
	@echo [${LOGFILE}] "--Done!"
shader:
	@echo [${LOGFILE}] "--Start Compilie Shader..."
	@${CXX} ${CXXFLAGS} ${SRC_PATH}/speedtree_shader.cpp -o ${OUT_PATH}/speedtree_shader.so ${INCLUDES} ${LIBS}
	@echo [${LOGFILE}] "--Done!"
ST_DSO:
	@echo [${LOGFILE}] "--Start Compilie ST_DSO..."
	@${CXX} ${CXXFLAGS} ${SRC_PATH}/maya/ST_DSO.cpp -o ${OUT_PATH}/ST_DSO.so ${INCLUDES} ${LIBS}
	@echo [${LOGFILE}] "--Done!"
install:
	@echo [${LOGFILE}] "--Make dir -> ${INSTALL_DIR}"
	@mkdir -p ${INSTALL_DIR}
	@mkdir -p ${INSTALL_DIR}/plugins
	@mkdir -p ${INSTALL_DIR}/plugins/Args
	@echo [${LOGFILE}] "--Install  ${OUT_PATH}/speedtree_procedural.so -> ${INSTALL_DIR}/plugins"
	@cp -f ${OUT_PATH}/speedtree_procedural.so ${INSTALL_DIR}/plugins
	@echo [${LOGFILE}] "--Install ${OUT_PATH}/speedtree_shader.so -> ${INSTALL_DIR}/plugins"
	@cp -f ${OUT_PATH}/speedtree_shader.so ${INSTALL_DIR}/plugins
	@echo [${LOGFILE}] "--Install speedtree_shader.args -> ${INSTALL_DIR}/plugins/Args"
	@cp -f ${SRC_PATH}/ui/speedtree_shader.args ${INSTALL_DIR}/plugins/Args
	@echo [${LOGFILE}] "--Install speedtree_procedural.mtd -> ${INSTALL_DIR}/plugins"
	@cp -f ${SRC_PATH}/ui/speedtree_shader.mtd ${INSTALL_DIR}/plugins
	@echo [${LOGFILE}] "--Install speedtree_shader.mtd -> ${INSTALL_DIR}/plugins"
	@cp -f ${SRC_PATH}/ui/speedtree_procedural.mtd ${INSTALL_DIR}/plugins
	@echo [${LOGFILE}] "--Install ${OUT_PATH}/ST_DSO.so -> ${INSTALL_DIR}/plugins"
	@cp -f ${OUT_PATH}/ST_DSO.so ${INSTALL_DIR}/plugins
	@echo [${LOGFILE}] "--Done!"
start:
	@echo [${LOGFILE}] "--SpeedtreeProc..."
	@mkdir -p ${OUT_PATH}