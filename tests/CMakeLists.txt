include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# Make all targets in this directory look for headers here
include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include)
# Make all targets in this directory link against Gtest
link_libraries(GTest::gtest_main)

# Add test executable
add_executable(MailTMTests ${CMAKE_SOURCE_DIR}/tests/mail_tm_tests.cpp)
target_link_libraries(MailTMTests PRIVATE MailTM CurlWrapper GTest::gtest GTest::gtest_main)
gtest_discover_tests(MailTMTests) 