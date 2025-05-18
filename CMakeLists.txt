cmake_minimum_required(VERSION 3.15)

project(Socket)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COMPILER_FLAGS "-g")

find_package(OpenCV "4.6.0" REQUIRED)

include_directories(
    include
    ${OpenCV_INCLUDE_DIRS}
    /usr/include/PHOENIX
)

add_executable(server src/SocketServer.cpp src/Message.cpp src/server.cpp)
add_executable(camerainfo_server src/SocketServer.cpp src/Message.cpp src/camerainfo_server.cpp)
add_executable(tf_server src/SocketServer.cpp src/Message.cpp src/tf.cpp)
add_executable(client src/SocketClient.cpp src/Message.cpp src/client.cpp)

target_link_libraries(server pthread ${OpenCV_LIBS} -lPHOENIX)
target_link_libraries(client pthread ${OpenCV_LIBS} -lPHOENIX)
target_link_libraries(camerainfo_server pthread ${OpenCV_LIBS} -lPHOENIX)
target_link_libraries(tf_server pthread ${OpenCV_LIBS} -lPHOENIX)