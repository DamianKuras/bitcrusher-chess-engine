add_executable(Uci uci.cpp)

target_link_libraries(Uci PRIVATE Engine)
target_include_directories(Uci PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# GA tournament builds write ga_pch.hpp here before configuring CMake;
# the precompiled header is picked up automatically when the file exists.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ga_pch.hpp")
    target_precompile_headers(Uci PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ga_pch.hpp")
endif()
