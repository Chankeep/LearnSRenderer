cmake_minimum_required (VERSION 3.0)
project(tinyrenderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OpenCV_DIR "D:/Tools/OpenCV")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

aux_source_directory(./source SOURCES)#将源文件列表写在变量SrcFiles中
set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/image)
add_executable(${PROJECT_NAME} ${SOURCES})

