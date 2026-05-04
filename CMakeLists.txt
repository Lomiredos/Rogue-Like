cmake_minimum_required(VERSION 3.20)
project(Rogue-Like VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(extern/eliott-engine)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(Rogue-Like ${SOURCES})


target_include_directories(Rogue-Like PRIVATE
    include/
)

target_link_libraries(Rogue-Like PRIVATE
    eliott-engine
)
