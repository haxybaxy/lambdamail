# Create executables
add_executable(get_domain get_domain.cpp)
add_executable(register_email register_email.cpp)
add_executable(check_inbox_with_curl check_inbox_with_curl.cpp)

# Common include directories for all targets
include_directories(
    ${JSONCPP_INCLUDE_DIR}
)

# Link libraries for get_domain
target_link_libraries(get_domain
    PRIVATE
    ${CURL_LIBRARIES}
    ${JSONCPP_LIBRARY}
)

# Include directories for register_email
target_include_directories(register_email
    PRIVATE
    ${CURL_LIBRARIES}
    ${JSONCPP_INCLUDE_DIR}
    ${RESTCLIENT_INCLUDE_DIR}
)

# Link libraries for register_email
target_link_libraries(register_email
    PRIVATE
    ${CURL_LIBRARIES}
    ${JSONCPP_LIBRARY}
    ${RESTCLIENT_LIBRARY}
)
# Link libraries for check_inbox_with_curl
target_link_libraries(check_inbox_with_curl
    PRIVATE
    ${CURL_LIBRARIES}
    ${JSONCPP_LIBRARY}

)
