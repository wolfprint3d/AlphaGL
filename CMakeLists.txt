cmake_minimum_required(VERSION 3.8)
project(AlphaGL)

include(mama.cmake)
include_directories(
    ${MAMA_INCLUDES}
    "."
)

if(WIN32)
    set(RUNTIME opengl32.lib)
elseif(LINUX)
    set(RUNTIME GL X11)
endif()

file(GLOB AGL_SOURCES AGL/*.cpp AGL/*.h AGL/*.c)
add_library(AGL ${AGL_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${AGL_SOURCES})

target_link_libraries(AGL PRIVATE ${MAMA_LIBS} PRIVATE ${RUNTIME})

set_property(TARGET AGL PROPERTY CXX_STANDARD 17)
set_property(TARGET AGL PROPERTY CXX_STANDARD_REQUIRED)

install(TARGETS AGL DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/lib)
