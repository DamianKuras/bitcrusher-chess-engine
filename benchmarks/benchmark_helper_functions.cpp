#include "benchmark_helper_functions.hpp"

#include <fstream>
#include <iostream>

namespace bench::utils {

std::string loadFENFromFile(const std::filesystem::path& file_path) {
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

namespace {
Epd stringToEPD(const std::string& line) {
    std::istringstream iss(line);
    std::string        token;
    int                field_count = 0;
    Epd                epd;
    while (iss >> token) {
        if (field_count < 4) {
            if (! epd.fen.empty()) {
                epd.fen += " ";
            }
            epd.fen += token;
            ++field_count;
        } else if (token == "bm") {
            iss >> epd.best_move;
            epd.best_move.pop_back(); // Remove ';' after best move.
        }
    }
    return epd;
}
} // namespace

[[nodiscard]] Epd loadEPDFromFile(const std::filesystem::path& file_path) {
    std::ifstream file(file_path);
    if (! file.is_open()) {
        throw std::runtime_error("Error: Unable to open file: " + file_path.string());
    }
    std::string line;
    if (! std::getline(file, line)) {
        throw std::runtime_error("Error: Unable to read from file: " + file_path.string());
    }
    return stringToEPD(line);
}

[[nodiscard]] std::vector<Epd> loadEPDsFromFile(const std::filesystem::path& file_path) {
    std::ifstream file(file_path);
    if (! file.is_open()) {
        throw std::runtime_error("Error: Unable to open file: " + file_path.string());
    }
    std::string      line;
    std::vector<Epd> epds;
    while (std::getline(file, line)) {
        epds.push_back(stringToEPD(line));
    }
    return epds;
}

} // namespace bench::utils