cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "video_player_aurora")
# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "video_player_aurora_platform_plugin")

project(${PROJECT_NAME} LANGUAGES CXX)

include(FetchContent)

add_library(${PLUGIN_NAME} SHARED
  "video_player_aurora_plugin.cpp"
  "video_player.cpp"
  "video_player.h"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB IMPORTED_TARGET REQUIRED glib-2.0)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED IMPORTED_TARGET gstreamer-app-1.0)
pkg_check_modules(GSTREAMER-AUDIO REQUIRED IMPORTED_TARGET gstreamer-audio-1.0)
pkg_check_modules(FlutterEmbedder REQUIRED IMPORTED_TARGET flutter-embedder)
pkg_check_modules(GLES REQUIRED IMPORTED_TARGET glesv2)

# if(USE_EGL_IMAGE_DMABUF)
# pkg_check_modules(GSTREAMER_GL REQUIRED gstreamer-gl-1.0)
# endif()

include_directories(
  ${GLIB_INCLUDE_DIRS}
  ${GSTREAMER_INCLUDE_DIRS}
  ${GSTREAMER-APP_INCLUDE_DIRS}
  ${GSTREAMER-AUDIO_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

link_directories(
  ${GLIB_LIBRARY_DIRS}
  ${GSTREAMER_LIBRARY_DIRS}
  ${GSTREAMER-APP_LIBRARY_DIRS}
  ${GSTREAMER-AUDIO_LIBRARY_DIRS}
)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-psabi")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set_target_properties(${PLUGIN_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
target_link_libraries(${PLUGIN_NAME} PRIVATE PkgConfig::FlutterEmbedder  PkgConfig::GLES)

target_link_libraries(${PLUGIN_NAME} PRIVATE PkgConfig::GSTREAMER)
target_link_libraries(${PLUGIN_NAME} PRIVATE PkgConfig::GLIB)

target_include_directories(${PLUGIN_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PLUGIN_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME})
target_include_directories(${PLUGIN_NAME} PUBLIC ${GSTREAMER_INCLUDE_DIRS})
target_include_directories(${PLUGIN_NAME} PUBLIC ${GLIB_INCLUDE_DIRS})
target_compile_definitions(${PLUGIN_NAME} PRIVATE PLUGIN_IMPL)

# if(USE_EGL_IMAGE_DMABUF)
# target_include_directories(${PLUGIN_NAME}
#   PRIVATE
#     ${GSTREAMER_GL_INCLUDE_DIRS}
# )
# endif()

# target_link_libraries(${PLUGIN_NAME}
#   PRIVATE
#     ${GLIB_LIBRARIES}
#     ${GSTREAMER_LIBRARIES}
# )
# if(USE_EGL_IMAGE_DMABUF)
# target_link_libraries(${PLUGIN_NAME}
#   PRIVATE
#     ${GSTREAMER_GL_LIBRARIES}
# )
# endif()

# List of absolute paths to libraries that should be bundled with the plugin
# set(video_player_aurora_bundled_libraries
#   ""
#   PARENT_SCOPE
# )