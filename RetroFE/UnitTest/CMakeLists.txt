cmake_minimum_required (VERSION 2.8)

project (RetroFEUnitTest)

# Setup testing
#add_subdirectory(gtest-1.7.0)
add_subdirectory(gmock-1.7.0)
enable_testing()

include_directories(../src ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

# Add test cpp file
add_executable(RunUnitTests_Setup
    testgtest.cpp
)

add_executable(RunUnitTests_Utility_Utils
	RetroFE/Utility/Utils_UnitTest.cpp
	../Source/Utility/Utils.cpp
)

# Link test executable against gtest & gtest_main
target_link_libraries(RunUnitTests_Setup gtest gtest_main)
target_link_libraries(RunUnitTests_Utility_Utils gtest gtest_main)

add_test(
    NAME RunUnitTests_Setup
    COMMAND RunUnitTests_Setup
)

add_test(
    NAME RunUnitTests_Util_Utils
    COMMAND RunUnitTests_Utility_Utils
)