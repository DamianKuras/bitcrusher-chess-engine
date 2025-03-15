#include "benchmark_helper_functions.hpp"

#include <fstream>

namespace bench::utils {

std::string loadFenFromFile(const std::filesystem::path& file_path) {
    std::ifstream file(file_path);
    if (! file.is_open()) {
        throw std::runtime_error("Error: Unable to open file: " + file_path.string());
    }
    std::string fen;
    if (! std::getline(file, fen)) {
        throw std::runtime_error("Error: Unable to read from file: " + file_path.string());
    }
    return fen;
}

} // namespace bench::utils