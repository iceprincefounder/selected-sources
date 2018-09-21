cmake_minimum_required (VERSION 3.2)
project(KatanaViewerModifierPlugins)

include(CheckSymbolExists)
include(CMakePushCheckState)

set(KATANA_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." CACHE PATH "Path to Katana")
list(INSERT CMAKE_MODULE_PATH 0 "${KATANA_ROOT}/plugins/Src/cmake")

set(CMAKE_CXX_STANDARD 11)

find_package(Katana PATHS "${KATANA_ROOT}/plugin_apis/cmake" REQUIRED)

if (NOT MSVC)
    add_compile_options(-Wall)
endif ()

# Ensure we don't compile DSOs with undefined symbols.
if (CMAKE_SYSTEM_NAME MATCHES Linux)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")
endif ()

find_package(OpenGL)

if (NOT OPENGL_FOUND)
    message(WARNING "Not compiling ViewerModifier plugins as OpenGL was not found.")
    return ()
endif ()

add_subdirectory(LightViewerModifier)
