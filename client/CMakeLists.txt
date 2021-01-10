cmake_minimum_required(VERSION 3.17)
project(client)

set(CMAKE_CXX_STANDARD 20)

set(SFML_INCLUDE_DIR "/home/Downloads/SFML-2.5.1/include")
set(SFML_LIBRARY_DIR "/home/Downloads/SFML-2.5.1/lib")
set(SFML_DIR "/home/Downloads/SFML-2.5.1/lib/cmake/SFML")

link_directories(SFML_LIBRARY_DIR)
include_directories(SFML_INCLUDE_DIR)

find_package(SFML 2.5.1 COMPONENTS system window graphics network audio)

if(SFML_FOUND)
    message(STATUS "SFML_INCLUDE_DIR: ${SFML_INCLUDE_DIR}")
    message(STATUS "SFML_LIBRARIES: ${SFML_LIBRARIES}")
    message(STATUS "SFML_VERSION: ${SFML_VERSION}")
endif()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/flav/Documents/chess/client")

set(SOURCE_FILES main.cpp)
add_executable(client ${SOURCE_FILES})

target_link_libraries(client sfml-graphics sfml-window sfml-system)