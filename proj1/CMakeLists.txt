# Check minimum CMake version
cmake_minimum_required(VERSION 3.10)

project(mandelbrot)

set(CMAKE_VERBOSE_MAKEFILE 1)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(USE_O3 "Compile with -O3 optimization" OFF)



if(USE_O3)    
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using GCC
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        # using icx
        set(CMAKE_CXX_FLAGS "-O3 -xhost -g -qopenmp-simd -qopt-report=1 -qopt-report-phase=vec")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
        # using icpx
        set(CMAKE_CXX_FLAGS "-O3 -xhost -g -qopenmp-simd -qopt-report=3")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # using Visual Studio C++
        # ...
    endif()

else()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -march=native -mtune=native")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using GCC
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -march=native -mtune=native")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        # using icx
        set(CMAKE_CXX_FLAGS "-O0 -xhost -g -qopenmp-simd -qopt-report=1 -qopt-report-phase=vec")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
        # using icpx
        set(CMAKE_CXX_FLAGS "-O0 -xhost -g -qopenmp-simd -qopt-report=3")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # using Visual Studio C++
        # ...
    endif()

endif()

find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIRS})

set(SOURCE_FILES
    calculators/BaseMandelCalculator.cc
    calculators/BatchMandelCalculator.cc
    calculators/LineMandelCalculator.cc
    calculators/RefMandelCalculator.cc
    common/cnpy.cc
    main.cc
)

include_directories(common)
include_directories(calculators)

add_executable(mandelbrot ${SOURCE_FILES})
target_link_libraries(mandelbrot ${ZLIB_LIBRARIES})
