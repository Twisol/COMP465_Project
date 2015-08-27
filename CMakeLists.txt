cmake_minimum_required(VERSION 3.3)
project(COMP465_Project)

set(COMP465_Project_VERSION 0.0.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -Werror -Wno-deprecated")
set(SOURCE_FILES
    main.cpp
    buildconf.h.in
)

# Funny hack to prevent GLUT from importing both gl.h and gl3.h
if(${APPLE})
  add_definitions(-D__gl_h -DGL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED)
endif()

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIR})

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})

# Add GLM
if(${APPLE})
  include_directories("/usr/local/Cellar/glm/0.9.6.3/include/")
endif()

configure_file(
    "${PROJECT_SOURCE_DIR}/buildconf.h.in"
    "${PROJECT_BINARY_DIR}/buildconf.h"
)
include_directories("${PROJECT_BINARY_DIR}")

add_executable(COMP465_Project ${SOURCE_FILES})
target_link_libraries(
    COMP465_Project
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${GLEW_LIBRARIES}
)