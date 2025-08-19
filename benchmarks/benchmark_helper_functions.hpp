#ifndef BENCHMARK_HELPER_FUNCTIONS_HPP
#define BENCHMARK_HELPER_FUNCTIONS_HPP

#include <filesystem>
#include <string>

namespace bench::utils {

struct Epd {
    std::string fen;
    std::string best_move;
};

// Load the first line from a given file. Used for loading single Forsyth-Edwards Notation (FEN)
// from file.
//
// throws std::runtime_error if the file cannot be opened or read
[[nodiscard]] std::string loadFENFromFile(const std::filesystem::path& file_path);

// Load the first line from a given file. Used for loading single Extended Position Description
// (EPD) from file.
//
// throws std::runtime_error if the file cannot be opened or read
[[nodiscard]] Epd loadEPDFromFile(const std::filesystem::path& file_path);
} // namespace bench::utils
#endif // BENCHMARK_HELPER_FUNCTIONS_HPP
