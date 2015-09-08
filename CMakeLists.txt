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


## Project configuration
set(COMP465_Project_VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic")
set(SOURCE_FILES
    shared.h
    main.cpp
    shaders.cpp
    App.cpp
    util/debug.cpp
    Mesh.cpp
    Entity.h
    Frame.h
)

# Create a build configuration header.
# This may be useful in the future, but for now it just adds the VERSION defined above.
configure_file(
    "${PROJECT_SOURCE_DIR}/shared.h.in"
    "${PROJECT_BINARY_DIR}/shared.h"
)
include_directories("${PROJECT_BINARY_DIR}")

add_executable(COMP465_Project ${SOURCE_FILES})
target_link_libraries(COMP465_Project ${STATIC_DEPENDENCIES})
