cmake_minimum_required (VERSION 3.0)
project(tinyrenderer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

aux_source_directory(./source SOURCE)#将源文件列表写在变量SrcFiles中
set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/image)
add_executable(${PROJECT_NAME} ${SOURCE})

