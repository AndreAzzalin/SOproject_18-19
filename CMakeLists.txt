cmake_minimum_required(VERSION 3.14)
project(SOproject_master)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

set(my_includes libs_structs.h jedi.c lib.h)


add_executable(yourExec1 ${my_includes} professor.c lib.h struct.h)
add_executable(yourExec2 ${my_includes} student.c lib.h)
add_executable(jedi jedi.c lib.h struct.h)
add_executable(padawan padawan.c lib.h struct.h)

