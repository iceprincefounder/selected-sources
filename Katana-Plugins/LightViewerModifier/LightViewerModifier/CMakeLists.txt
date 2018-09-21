add_library(LightViewerModifier MODULE src/LightViewerModifier.cpp)

target_include_directories(LightViewerModifier PRIVATE include)

target_link_libraries(LightViewerModifier
    PRIVATE
    Katana::FnViewerModifierPlugin
    Katana::FnAttribute
    Katana::FnPlatform
)

target_include_directories(LightViewerModifier PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(LightViewerModifier PRIVATE ${OPENGL_LIBRARIES})

set_target_properties(LightViewerModifier PROPERTIES PREFIX "")
install(TARGETS LightViewerModifier DESTINATION Libs)
