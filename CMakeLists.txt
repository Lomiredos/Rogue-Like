cmake_minimum_required(VERSION 3.20)
project(Rogue-Like VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "C:/Dev/vcpkg/installed/x64-windows")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(extern/eliott-engine)

find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(Rogue-Like ${SOURCES})


target_include_directories(Rogue-Like PRIVATE
    include/
)

target_link_libraries(Rogue-Like PRIVATE
    eliott-engine
    nlohmann_json::nlohmann_json
)

add_custom_command(TARGET Rogue-Like POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Rogue-Like>/assets
  )