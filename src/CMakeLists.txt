# Find SFML package
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Add this near the top after finding SFML
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add this near the top of the file
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")

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

# Create the CurlWrapper library
add_library(CurlWrapper STATIC CurlWrapper.cpp)
target_include_directories(CurlWrapper PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(CurlWrapper PRIVATE CURL::libcurl)

# Link CurlWrapper to MailTM
target_link_libraries(MailTM PRIVATE CurlWrapper)

# Create the GUI library
add_library(EmailClientGUI STATIC EmailClientGUI.cpp)
target_link_libraries(EmailClientGUI
    PRIVATE
    MailTM
    sfml-graphics
    sfml-window
    sfml-system
)

# When creating EmailClientGUI target, add include directories
target_include_directories(EmailClientGUI
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${JSONCPP_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Create executables
add_executable(mail_client main.cpp)
target_link_libraries(mail_client
    PRIVATE
    EmailClientGUI
    MailTM
    sfml-graphics
    sfml-window
    sfml-system
)

# Add this after your other add_executable commands
add_custom_command(
    TARGET mail_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${RESOURCES_DIR} ${CMAKE_BINARY_DIR}/resources
)

# Optional: Keep the other executables if you still want them
add_executable(get_domain get_domain.cpp)
add_executable(register_email register_email.cpp)
add_executable(check_inbox check_inbox.cpp)

# Common include directories for all executables
include_directories(
    ${JSONCPP_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)

# Link MailTM to the other executables
target_link_libraries(get_domain PRIVATE MailTM)
target_link_libraries(register_email PRIVATE MailTM)
target_link_libraries(check_inbox PRIVATE MailTM)
