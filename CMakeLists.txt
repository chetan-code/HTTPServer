cmake_minimum_required(VERSION 3.10)

project(HTTPServer)

# use c++ 17 (used structured binding)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#-I
include_directories(include)

# add executables
add_executable(server
    src/main.cpp 
    src/TcpServer.cpp
    src/ThreadPool.cpp
)

#link winsock cd..
if(WIN32)
    target_link_libraries(server ws2_32)
endif()