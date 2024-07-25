# CMAKE_SYSTEM_PROCESSOR - CPU architecture name (armv7l, aarch64, x86_64)
# CMAKE_BUILD_TYPE       - Build mode (debug, profile, release)
# PSDK_VERSION           - Build psdk exact version
# PSDK_MAJOR             - Build psdk major version

cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME mobile_scanner)
set(PLUGIN_NAME  mobile_scanner_platform_plugin)

project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-psabi")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(3RDPATRY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rdpatry/psdk_${PSDK_MAJOR})

add_compile_definitions(PSDK_MAJOR=${PSDK_MAJOR})

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Multimedia REQUIRED)

pkg_check_modules(FlutterEmbedder REQUIRED IMPORTED_TARGET flutter-embedder)
pkg_check_modules(GLES REQUIRED IMPORTED_TARGET glesv2)
pkg_check_modules(SC REQUIRED IMPORTED_TARGET streamcamera)

add_library(${PLUGIN_NAME} SHARED mobile_scanner_plugin.cpp texture_camera.cpp)

#################### yuv
set(YUV_LIB_NAME "libyuv")
set(YUV_LIB_LINK "libyuv.so.0")
set(YUV_LIB_LIST ${YUV_LIB_LINK} "libyuv.so.0.0.0")

target_include_directories(${PLUGIN_NAME} PUBLIC ${3RDPATRY_PATH}/${YUV_LIB_NAME}/include)
target_include_directories(${PLUGIN_NAME} PUBLIC ${3RDPATRY_PATH}/${YUV_LIB_NAME}/include/${YUV_LIB_NAME})

foreach (file ${YUV_LIB_LIST})
    add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${3RDPATRY_PATH}/${YUV_LIB_NAME}/${CMAKE_SYSTEM_PROCESSOR}/${file}
                    ${ROOT_PROJECT_BINARY_DIR}/bundle/lib/${file})
endforeach()

add_library(${YUV_LIB_NAME} SHARED IMPORTED)
set_property(TARGET ${YUV_LIB_NAME} PROPERTY IMPORTED_LOCATION ${3RDPATRY_PATH}/${YUV_LIB_NAME}/${CMAKE_SYSTEM_PROCESSOR}/${YUV_LIB_LINK})
target_link_libraries(${PLUGIN_NAME} PUBLIC ${YUV_LIB_NAME})
####################

################### zxing
set(ZXING_LIB_NAME "ZXing")

if(${PSDK_MAJOR} EQUAL 5)
    set(ZXING_LIB_LINK "libZXing.so.3")
    set(ZXING_LIB_LIST ${ZXING_LIB_LINK} "libZXing.so.2.1.0")
else()
    set(ZXING_LIB_LINK "libZXing.so.1")
    set(ZXING_LIB_LIST ${ZXING_LIB_LINK} "libZXing.so.1.1.1")
endif()

target_include_directories(${PLUGIN_NAME} PUBLIC ${3RDPATRY_PATH}/${ZXING_LIB_NAME}/include)
target_include_directories(${PLUGIN_NAME} PUBLIC ${3RDPATRY_PATH}/${ZXING_LIB_NAME}/include/${YUV_LIB_NAME})

foreach (file ${ZXING_LIB_LIST})
    add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${3RDPATRY_PATH}/${ZXING_LIB_NAME}/${CMAKE_SYSTEM_PROCESSOR}/${file}
                    ${ROOT_PROJECT_BINARY_DIR}/bundle/lib/${file})
endforeach()

add_library(${ZXING_LIB_NAME} SHARED IMPORTED)
set_property(TARGET ${ZXING_LIB_NAME} PROPERTY IMPORTED_LOCATION ${3RDPATRY_PATH}/${ZXING_LIB_NAME}/${CMAKE_SYSTEM_PROCESSOR}/${ZXING_LIB_LINK})
target_link_libraries(${PLUGIN_NAME} PUBLIC ${ZXING_LIB_NAME})
###################

set_target_properties(${PLUGIN_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden AUTOMOC ON)

target_link_libraries(${PLUGIN_NAME} PRIVATE PkgConfig::FlutterEmbedder PkgConfig::GLES PkgConfig::SC)
target_link_libraries(${PLUGIN_NAME} PUBLIC Qt5::Core Qt5::Multimedia)

target_include_directories(${PLUGIN_NAME} PRIVATE ${FLUTTER_DIR})
target_include_directories(${PLUGIN_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PLUGIN_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME})

target_compile_definitions(${PLUGIN_NAME} PRIVATE PLUGIN_IMPL)
