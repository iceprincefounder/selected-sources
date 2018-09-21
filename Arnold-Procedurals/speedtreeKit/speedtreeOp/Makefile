# See README for instructions on how to use this Makefile

KATANA_HOME=/mnt/work/software/katana/katana2.5v5

# The output objects dir
OBJDIR = ./out
INSTALLDIR = ../Release/third_party/Katana2.5
REGISTER_PY=./source/*.py

OUTFILENAME = SpeedTreeIn.so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

# Plug-in sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES = source/*.cpp

SOURCES += $(shell find $(PLUGIN_SRC)/FnAttribute/client -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/op -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/util -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/pystring -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnPluginManager/client -name \*.cpp)
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnGeolibCookInterfaceUtilsService.cpp
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnXFormUtil.cpp
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnBuiltInOpArgsUtil.cpp

SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnAttributeFunctionUtil.cpp
SOURCES += $(PLUGIN_SRC)/FnGeolibServices/client/FnArgsFile.cpp

SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
SOURCES += $(PLUGIN_SRC)/FnRendererInfo/client/FnRendererInfoPluginClient.cpp

SOURCES += $(PLUGIN_SRC)/FnAttributeFunction/plugin/FnAttributeFunctionPlugin.cpp

SOURCES += $(shell find $(PLUGIN_SRC)/FnDefaultAttributeProducer/plugin/ -name \*.cpp)

INCLUDES = -I$(KATANA_HOME)/plugin_apis/include

CFLAGS=-ffloat-store
CXXFLAGS=-Wall -Wextra -Wshadow -Wconversion -Wcast-qual -fPIC -DPIC

# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

LIBPATH = -L$(KATANA_HOME)/bin/Geolib3/internal/CEL
LIBPATH += -L$(KATANA_HOME)/bin

LIBS = -lCEL

# Targets:
all: $(OUTFILEPATH)

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling SphereMaker"
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "  Cleaning SphereMaker"
	@rm -rf $(OBJDIR)

install:
	@echo "  Installing to $(INSTALLDIR)"
	@mkdir -p $(INSTALLDIR)/Libs
	@mkdir -p $(INSTALLDIR)/Plugins
	@cp $(OUTFILEPATH) $(INSTALLDIR)/Libs
	@cp $(REGISTER_PY) $(INSTALLDIR)/Plugins
