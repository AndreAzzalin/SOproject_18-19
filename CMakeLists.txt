cmake_minimum_required(VERSION 3.14)
project(SOproject_master)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

set(my_includes  gestore.c lib.h)


add_executable(yourExec1 ${my_includes} gestore.c lib.h )
add_executable(yourExec2 ${my_includes} student.c lib.h)
add_executable(gestore gestore.c lib.h )
add_executable(student student.c lib.h )

