cmake_minimum_required(VERSION 3.26)
project(ppl)

set(CMAKE_CXX_STANDARD 23)

add_executable(ppl
        Queue.hpp
        FarmManager.hpp
        SequentialManager.hpp
        Timer.hpp
        PipelineManager.hpp
        Node.hpp
        EOS.cpp
        EOS.hpp
        Examples/farm_test.cpp
#        Examples/pipeline_test2.cpp
)
