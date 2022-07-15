cmake_minimum_required (VERSION 3.0.0)#规定cmake的最低版本要求
project(tinyRenderer)#项目的名称，不一定和你的文件夹名称一
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER "g++")
set(CMAKE_CXX_COMPILER "g++")
 
include_directories(${PROJECT_SOURCE_DIR}/source)#添加头文件的搜索路径
aux_source_directory(./source SrcFiles)#将源文件列表写在变量SrcFiles中
set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/image)#设置可执行文件输出路径
add_executable(tinyRenderer ${SrcFiles})#设置可执行文件的名称，make之后bin目录下出现main.exe