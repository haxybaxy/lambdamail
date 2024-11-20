# Create the MailTM library
add_library(MailTM STATIC MailTM.cpp)

# Include directories for the MailTM library
target_include_directories(MailTM
    PRIVATE
    ${JSONCPP_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Link libraries for the MailTM library
target_link_libraries(MailTM
    PRIVATE
    ${CURL_LIBRARIES}
    ${JSONCPP_LIBRARY}
)

# Create executables for the project
add_executable(get_domain get_domain.cpp)
add_executable(register_email register_email.cpp)
add_executable(check_inbox check_inbox.cpp)

# Common include directories for all executables
include_directories(
    ${JSONCPP_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Link and include the MailTM library for get_domain
target_link_libraries(get_domain
    PRIVATE
    MailTM
)

# Link and include the MailTM library for register_email
target_link_libraries(register_email
    PRIVATE
    MailTM
)

# Link and include the MailTM library for check_inbox
target_link_libraries(check_inbox
    PRIVATE
    MailTM
)
