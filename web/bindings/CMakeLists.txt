find_program(PYTHON_CMD NAMES python python3 REQUIRED)

execute_process(
    COMMAND "${PYTHON_CMD}" -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
file(TO_CMAKE_PATH "${pybind11_DIR}" pybind11_DIR)

# Force pybind11 to use the modern FindPython strategy (avoids deprecated warning)
set(PYBIND11_FINDPYTHON ON)

# Force CMake to use the exact native Python executable that `python` currently points to, 
# instead of letting find_package stumble into an incompatible MSYS/MinGW python3.exe on the PATH.
execute_process(
    COMMAND "${PYTHON_CMD}" -c "import sys; print(sys.executable)"
    OUTPUT_VARIABLE PYTHON_EXE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
file(TO_CMAKE_PATH "${PYTHON_EXE}" PYTHON_EXE)
set(Python_EXECUTABLE "${PYTHON_EXE}" CACHE FILEPATH "")

find_package(pybind11 REQUIRED PATHS "${pybind11_DIR}")

pybind11_add_module(chess_engine chess_module.cpp)

target_link_libraries(chess_engine PRIVATE Engine)

# Output the .so next to the FastAPI app so `import chess_engine` works.
set_target_properties(chess_engine PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/web"
)
