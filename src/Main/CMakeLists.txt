file(GLOB SOURCES "*.ui" "*.cpp" "*.h" "../*.qrc" "./darkstyle/*.qrc" "./skin/*.qrc") 
#将ui文件和生成文件整理在一个文件夹中，非必须
SOURCE_GROUP("Main" FILES ${SOURCES})

#将源文件资源编译为可执行文件
add_executable(main ${SOURCES})

#添加程序所需要的的链接库
target_link_libraries(main Qt5::Core Qt5::Widgets Qt5::Gui io base)