project(SummerFramework)
cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(main main.cpp)
target_link_libraries(main ${CONAN_LIBS})

