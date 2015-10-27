cmake_minimum_required(VERSION 3.2)
project(COMP465_Project)

## Dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeFiles/Modules/")


find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
message(STATUS "OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")
set(STATIC_DEPENDENCIES "${STATIC_DEPENDENCIES};${OPENGL_LIBRARIES}")

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})
message(STATUS "GLEW_LIBRARIES: ${GLEW_LIBRARIES}")
set(STATIC_DEPENDENCIES "${STATIC_DEPENDENCIES};${GLEW_LIBRARIES}")

find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIR})
message(STATUS "GLFW_LIBRARIES: ${GLFW_LIBRARIES}")
set(STATIC_DEPENDENCIES "${STATIC_DEPENDENCIES};${GLFW_LIBRARIES}")

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIR})
message(STATUS "GLM_LIBRARIES: ${GLM_LIBRARIES}")
set(STATIC_DEPENDENCIES "${STATIC_DEPENDENCIES};${GLM_LIBRARIES}")

# Funny hack to prevent GLUT from including both gl.h and gl3.h
if(${APPLE})
  # __gl_h is an include guard; setting it prevents gl.h from being included.
  # The long one suppresses the include-conflict error that will still
  # appear when __gl_h is defined.
  add_definitions(
      -D__gl_h
      -DGL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
  )
endif()

# Tell GLEW we plan to link with it statically.
add_definitions(-DGLEW_STATIC)

# Add some definitions for our own code to use
add_definitions(-DVERSION="0.0.2")
add_definitions(-DGL_VALIDATE_SHADERS)


## Project configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror -pedantic")
set(SOURCE_FILES
    main.cpp
    shaders.cpp
    App.cpp
    util/debug.cpp
    Mesh.cpp
)

add_executable(COMP465_Project ${SOURCE_FILES})
target_link_libraries(COMP465_Project ${STATIC_DEPENDENCIES})
