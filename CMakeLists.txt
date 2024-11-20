cmake_minimum_required(VERSION 3.10)
project(LambdaMail VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Get Conda environment path
if(DEFINED ENV{CONDA_PREFIX})
    set(CONDA_PREFIX $ENV{CONDA_PREFIX})
else()
    message(FATAL_ERROR "CONDA_PREFIX environment variable not set. Are you in a conda environment?")
endif()

# Find required packages
find_package(CURL REQUIRED)

# Set JsonCpp paths using conda environment
set(JSONCPP_INCLUDE_DIR "${CONDA_PREFIX}/include")
set(JSONCPP_LIBRARY "${CONDA_PREFIX}/lib/libjsoncpp.dylib")

# Set RestClient paths using conda environment
set(RESTCLIENT_INCLUDE_DIR "${CONDA_PREFIX}/include")
set(RESTCLIENT_LIBRARY "${CONDA_PREFIX}/lib/librestclient-cpp.dylib")

# Verify paths exist
if(NOT EXISTS "${RESTCLIENT_INCLUDE_DIR}/restclient-cpp/restclient.h")
    message(FATAL_ERROR "restclient.h not found at ${RESTCLIENT_INCLUDE_DIR}/restclient-cpp/restclient.h. Please check if restclient-cpp is installed correctly in your conda environment")
endif()

if(NOT EXISTS "${JSONCPP_INCLUDE_DIR}/json/json.h")
    message(FATAL_ERROR "json.h not found at ${JSONCPP_INCLUDE_DIR}/json/json.h. Please check if jsoncpp is installed correctly in your conda environment")
endif()

# Print paths for debugging
message(STATUS "Conda prefix: ${CONDA_PREFIX}")
message(STATUS "JsonCpp include: ${JSONCPP_INCLUDE_DIR}")
message(STATUS "JsonCpp library: ${JSONCPP_LIBRARY}")
message(STATUS "RestClient include: ${RESTCLIENT_INCLUDE_DIR}")
message(STATUS "RestClient library: ${RESTCLIENT_LIBRARY}")

# Add the src directory
add_subdirectory(src)
