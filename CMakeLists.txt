cmake_minimum_required(VERSION 3.10)
project(LambdaMail VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
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

# Verify JsonCpp paths
if(NOT EXISTS "${JSONCPP_INCLUDE_DIR}/json/json.h")
    message(FATAL_ERROR "json.h not found at ${JSONCPP_INCLUDE_DIR}/json/json.h. Please check if jsoncpp is installed correctly in your conda environment")
endif()

# Print paths for debugging
message(STATUS "Conda prefix: ${CONDA_PREFIX}")
message(STATUS "JsonCpp include: ${JSONCPP_INCLUDE_DIR}")
message(STATUS "JsonCpp library: ${JSONCPP_LIBRARY}")

# Include directories for headers
include_directories(
    ${JSONCPP_INCLUDE_DIR}
    ${CONDA_PREFIX}/include
    include
)

# Add the library for MailTM and CurlWrapper
add_library(MailTM STATIC src/MailTM.cpp)
add_library(CurlWrapper STATIC src/CurlWrapper.cpp)

# Link the required libraries to MailTM and CurlWrapper
target_link_libraries(MailTM PRIVATE CURL::libcurl ${JSONCPP_LIBRARY})
target_link_libraries(CurlWrapper PRIVATE CURL::libcurl)

# Add executables for get_domain, register_email, and check_inbox
add_executable(get_domain src/get_domain.cpp)
add_executable(register_email src/register_email.cpp)
add_executable(check_inbox src/check_inbox.cpp)

# Link MailTM and CurlWrapper to executables
target_link_libraries(get_domain MailTM CurlWrapper)
target_link_libraries(register_email MailTM CurlWrapper)
target_link_libraries(check_inbox MailTM CurlWrapper)
