file(GLOB HEADER *.h)
file(GLOB SOURCE *.cpp)
SOURCE_GROUP("header" FILES ${HEADER})
SOURCE_GROUP("source" FILES ${SOURCE})

add_library(pointcloud ${HEADER} ${SOURCE})

target_link_libraries(pointcloud base)