#指定cmake最小版本
cmake_minimum_required(VERSION 3.10)

#指定项目名称
project(PointCloud)

#使用的标准版本
set(CMAKE_CXX_STANDARD 11) 

#if中是忽略了常见的一些无用警告和bug
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP /wd4996 /wd4819 /wd4251 /wd4267 /wd4244 /wd4275 /wd4005 /wd5033 /wd4018")
    add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES -DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_ENABLE_EXTENDED_ALIGNED_STORAGE) # windows use windows api
endif(MSVC)

#设置工程包含当前目录，非必须
SET(CMAKE_INCLUDE_CURRENT_DIR ON) 

#设置编译时 将所有dll和exe放入bin目录下，将所有lib放入lib目录下。 可选
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# 设置自动生成moc文件,AUTOMOC打开可以省去QT5_WRAP_CPP命令
SET(CMAKE_AUTOMOC ON)
# 设置自动生成ui.h文件,AUTOUIC打开可以省去QT5_WRAP_UI命令
SET(CMAKE_AUTOUIC ON)
#设置自动生成rcc资源文件
SET(CMAKE_AUTORCC ON)

#通过Ui文件生成对应的头文件，一定要添加
# QT5_WRAP_UI(WRAP_FILES ${UI_FILES})

#find_package 获取QT5中的Core、Gui等组件
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets )

#添加矩阵运算库eigen
find_package(Eigen3 REQUIRED)

# Laszip3::Laszip3
find_package(Laszip3 REQUIRED)

#添加头文件搜索目录 Eigen3头文件目录 当前cmakelists文件所在目录/src
include_directories(${EIGEN3_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)

#添加子目录
add_subdirectory(src)