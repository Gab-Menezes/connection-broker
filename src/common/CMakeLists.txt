cmake_minimum_required(VERSION 3.16.3)

project(common VERSION 1.0)

add_library(CommonImpl 
                connection.h
                connection.cpp
                ts_queue.h
                ts_queue.cpp
                ts_vector.h
                ts_vector.cpp
                msg.h
                msg.cpp
            )

set_target_properties(CommonImpl PROPERTIES LINKER_LANGUAGE CXX)

include_directories(../../libs)
