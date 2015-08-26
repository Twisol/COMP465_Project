cmake_minimum_required(VERSION 3.3)
project(COMP465_Project)

set(COMP465_Project_VERSION 0.0.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -Werror -Wno-deprecated")
set(SOURCE_FILES
    main.cpp
    buildconf.h.in
)

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIR})

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})

configure_file(
    "${PROJECT_SOURCE_DIR}/buildconf.h.in"
    "${PROJECT_BINARY_DIR}/buildconf.h"
)
include_directories("${PROJECT_BINARY_DIR}")


#/usr/local/Cellar/freeglut/2.8.1/include
#/usr/local/Cellar/freeglut/2.8.1/lib/libglut.a

add_executable(COMP465_Project ${SOURCE_FILES})
target_link_libraries(COMP465_Project ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})