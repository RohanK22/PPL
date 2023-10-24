cmake_minimum_required(VERSION 3.26)
project(ppl)

set(CMAKE_CXX_STANDARD 23)

add_executable(ppl main.cpp
        Task.hpp
        Queue.hpp
        FarmManager.hpp
        thread_args.cpp
        thread_args.hpp
)
