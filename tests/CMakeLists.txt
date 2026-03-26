find_package(GTest REQUIRED)

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp")

add_executable(Tests ${TEST_SOURCES})

target_link_libraries(Tests PRIVATE Engine GTest::gtest GTest::gtest_main)
target_include_directories(Tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
