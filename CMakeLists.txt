cmake_minimum_required(VERSION 3.23)

project(MySdlGame)
find_package(SDL3 REQUIRED)
add_executable(main main.c)
target_link_libraries(main PRIVATE SDL3::SDL3 m)
