cmake_minimum_required(VERSION 3.26)
project(ppl)

set(CMAKE_CXX_STANDARD 23)

# Common source files
set(COMMON_SOURCES
        Queue.hpp
        FarmManager.hpp
        SequentialManager.hpp
        Timer.hpp
        PipelineManager.hpp
        Node.hpp
        EasyBMP.hpp
)

# Executable target: ppl
add_executable(ppl
        ${COMMON_SOURCES}
        Examples/mandelbrot.cpp
)

# Executable target: mandelbrot
add_executable(mandelbrot
        ${COMMON_SOURCES}
        Examples/mandelbrot.cpp
)

# Executable target: mandelbrot_seq
add_executable(mandelbrot_seq
        ${COMMON_SOURCES}
        Examples/mandelbrot_seq.cpp
)

# Executable target: julia_seq
add_executable(julia_seq
        ${COMMON_SOURCES}
        Examples/julia_seq.cpp
)

# Executable target: pipeline_test
add_executable(pipeline_test
        ${COMMON_SOURCES}
        Examples/pipeline_test2.cpp
)
