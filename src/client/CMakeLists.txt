cmake_minimum_required(VERSION 3.16.3)

project(client VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_executable(${PROJECT_NAME} main.cpp Client.h Client.cpp)
include_directories(../../libs)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

if (NOT TARGET CommonImpl)
    add_subdirectory(../common ../common)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE CommonImpl)
