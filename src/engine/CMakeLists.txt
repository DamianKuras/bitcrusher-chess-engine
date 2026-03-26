add_library(Engine INTERFACE)

target_include_directories(Engine INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/attack_generators"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/legal_move_generators"
)

target_compile_features(Engine INTERFACE cxx_std_23)

if(BITCRUSHER_WITH_BMI2)
    target_compile_definitions(Engine INTERFACE HAS_BMI2)
    if(MSVC)
        target_compile_options(Engine INTERFACE /arch:AVX2)
    else()
        target_compile_options(Engine INTERFACE -mbmi2)
    endif()
endif()

if(UNIX)
    target_link_libraries(Engine INTERFACE atomic)
endif()
