cmake_minimum_required(VERSION 3.26)
project(ppl)

set(CMAKE_CXX_STANDARD 23)

# Boost Config
set(Boost_INCLUDE_DIRS /Users/rohan/Documents/libs/boost_1_82_0/boost)
set(Boost_USE_STATIC_LIBS ON)  # If you prefer static linking
set(Boost_USE_MULTITHREADED ON)
set(Boost_LIBRARIES
        /Users/rohan/Documents/libs/boost_1_82_0/lib/libboost_mpi.a
        /Users/rohan/Documents/libs/boost_1_82_0/lib/libboost_serialization.a
        # ... other required Boost libraries if needed
)

# Include boost build directory as /cs/home/rk76/usr


# Common source files
set(COMMON_SOURCES
        Queue.hpp
        FarmManager.hpp
        SequentialManager.hpp
        Timer.hpp
        PipelineManager.hpp
        Node.hpp
        EasyBMP.hpp
        MPIFarmManager.hpp
        MPIPipelineManager.hpp
)

# Executable target: farm_test
add_executable(farm_test
        ${COMMON_SOURCES}
        Examples/farm_test.cpp
)

# Executable target: julia
add_executable(julia
        ${COMMON_SOURCES}
        Examples/julia.cpp
)

# Executable target: julia_seq
add_executable(julia_seq
        ${COMMON_SOURCES}
        Examples/julia_seq.cpp
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

# Executable target: pipeline_test
add_executable(pipeline_test
        ${COMMON_SOURCES}
        Examples/pipeline_test.cpp
)

# Executable target: pipeline_text_processing
add_executable(pipeline_text_processing
        ${COMMON_SOURCES}
        Examples/pipeline_text_processing.cpp
)

# Executable target: gaussian_blur
add_executable(gaussian_blur
        ${COMMON_SOURCES}
        Examples/gaussian_blur.cpp
)

# Executable target: farm_matmul
add_executable(farm_matmul
        ${COMMON_SOURCES}
        Examples/farm_matmul.cpp
)

# Executable target: seq_matmul
add_executable(seq_matmul
        ${COMMON_SOURCES}
        Examples/seq_matmul.cpp
)

# Executable target: farm_dot_product
add_executable(farm_dot_product
        ${COMMON_SOURCES}
        Examples/farm_dot_product.cpp
)

# Executable target: seq_dot_product
add_executable(seq_dot_product
        ${COMMON_SOURCES}
        Examples/seq_dot_product.cpp
)

# Executable target: farm_monte_carlo
add_executable(farm_monte_carlo
        ${COMMON_SOURCES}
        Examples/farm_monte_carlo.cpp
)

# Executable target: seq_monte_carlo
add_executable(seq_monte_carlo
        ${COMMON_SOURCES}
        Examples/seq_monte_carlo.cpp
)

# Nested Farm in Pipeline
# nexted_farm_in_pipeline_gaussian_blur
add_executable(nested_farm_in_pipeline_gaussian_blur
        ${COMMON_SOURCES}
        Examples/nested_farm_in_pipeline_gaussian_blur.cpp
)

# Boost MPI test
include_directories(${Boost_INCLUDE_DIRS})
find_package(Boost 1.82.0 REQUIRED COMPONENTS mpi serialization)
add_executable(boost_mpi_test
        ${COMMON_SOURCES}
        MPI_Examples/boost_mpi_test.cpp
)
target_link_libraries(boost_mpi_test
        ${Boost_LIBRARIES}
        MPI::MPI_CXX
)

# MPI Examples
# Mandelbrot
include_directories(${Boost_INCLUDE_DIRS})
find_package(Boost 1.82.0 REQUIRED COMPONENTS mpi serialization)
add_executable(mpi_mandelbrot
        ${COMMON_SOURCES}
        MPI_Examples/mandelbrot.cpp
)
target_link_libraries(mpi_mandelbrot
        ${Boost_LIBRARIES}
        MPI::MPI_CXX
)

# pipline_test
include_directories(${Boost_INCLUDE_DIRS})
find_package(Boost 1.82.0 REQUIRED COMPONENTS mpi serialization)
add_executable(mpi_pipeline_test
        ${COMMON_SOURCES}
        MPI_Examples/pipeline_test.cpp
)
target_link_libraries(mpi_pipeline_test
        ${Boost_LIBRARIES}
        MPI::MPI_CXX
)

# farm_test
include_directories(${Boost_INCLUDE_DIRS})
find_package(Boost 1.82.0 REQUIRED COMPONENTS mpi serialization)
add_executable(mpi_farm_test
        ${COMMON_SOURCES}
        MPI_Examples/farm_test.cpp
)
target_link_libraries(mpi_farm_test
        ${Boost_LIBRARIES}
        MPI::MPI_CXX
)

# Nested Farm in MPI Farm
# farm_test -- Factorials
include_directories(${Boost_INCLUDE_DIRS})
find_package(Boost 1.82.0 REQUIRED COMPONENTS mpi serialization)
add_executable(nested_farm_in_mpi_farm
        ${COMMON_SOURCES}
        MPI_Examples/nested_farm_in_mpi_farm.cpp
)
target_link_libraries(nested_farm_in_mpi_farm
        ${Boost_LIBRARIES}
        MPI::MPI_CXX
)

# Nested Farm in MPI Farm -- Montecarlo PI
include_directories(${Boost_INCLUDE_DIRS})
find_package(Boost 1.82.0 REQUIRED COMPONENTS mpi serialization)
add_executable(nested_farm_in_mpi_farm_monte_carlo
        ${COMMON_SOURCES}
        MPI_Examples/monte_carlo_pi.cpp
)
target_link_libraries(nested_farm_in_mpi_farm_monte_carlo
        ${Boost_LIBRARIES}
        MPI::MPI_CXX
)

#mpic++ -I/path/to/boost/mpi my_application.cpp -Llibdir \
#  -lboost_mpi -lboost_serialization
