cmake_minimum_required (VERSION 3.0.0)#规定cmake的最低版本要求
project(tinyRenderer)#项目的名称，不一定和你的文件夹名称一
set(OpenCV_DIR "D:/Tools/OpenCV")
find_package(OpenCV REQUIRED)
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/source)#添加头文件的搜索路径
include_directories("D:/Tools/Eigen3/include/eigen3")
include_directories(${OpenCV_INCLUDE_DIRS})

aux_source_directory(./source SrcFileS)#将源文件列表写在变量SrcFiles中
set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/image)#设置可执行文件输出路径
add_executable(tinyRenderer ${SrcFileS})#设置可执行文件的名称，make之后bin目录下出现main.exe
target_link_libraries(tinyRenderer ${OpenCV_LIBS})