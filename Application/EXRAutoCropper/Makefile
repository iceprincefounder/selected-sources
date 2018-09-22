
INSTALL_DIR=.

CXX=g++

EXR_INCLUDES="-I/mnt/work/software/develop/dependences/open_exr/include/OpenEXR"
EXR_LIBDIR="-L/mnt/work/software/develop/dependences/open_exr/lib"

OPT=-o3

EXRLIBS=$(EXR_LIBDIR) -Bstatic -lIex -lIlmImf -lIlmThread -lImath -lIex -lHalf -Bdynamic

ARCH = $(shell uname)
ifeq ($(ARCH),Linux)
  EXRLIBS += -lpthread
endif
ifeq ($(ARCH),OpenBSD)
  EXRLIBS += -lpthread
endif
ifeq ($(ARCH),Darwin)
  EXRLIBS += -lz
endif

INCLUDE=$(EXR_INCLUDES)
LIBS=$(EXR_LIBDIR) $(EXRLIBS) -lm 

WARN=-Wall
CXXFLAGS=$(OPT) $(MARCH) $(INCLUDE) $(WARN) $(DEFS)

SRC_DIR := ./src
OBJ_DIR := ./out
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

.PHONY: clean default

default: main

LOGFILE=$(LOGPATH) `date +'%H:%M:%S'`

clean: ; rm -f OBJ_FILES

start: ; @echo [${LOGFILE}] "--Start to comple...!"

main: start $(OBJ_FILES)
	@$(CXX) $(CXXFLAGS) -o ${OBJ_DIR}/EXRDataAutoCrop $(LIBS) $(OBJ_FILES)
	@echo [${LOGFILE}] "--Finished!"
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo [${LOGFILE}] "--Build $< -> $@"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

install:
	@echo [${LOGFILE}] "--Install..."
	@cp -f ${OBJ_DIR}/EXRDataAutoCrop inst/bin/EXRDataAutoCrop
	@#exec ./inst/test.crypto.sh
	@echo [${LOGFILE}] "--Done!"