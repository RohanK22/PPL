cmake_minimum_required(VERSION 3.26)
project(ppl)

set(CMAKE_CXX_STANDARD 23)

add_executable(ppl Examples/pipeline_test.cpp
        Task.hpp
        Queue.hpp
        FarmManager.hpp
        SequentialManager.hpp
        Timer.hpp
        PipelineManager.hpp
        Node.hpp
        thread_args.cpp
        thread_args.hpp
        EOS.cpp
        EOS.hpp
)
