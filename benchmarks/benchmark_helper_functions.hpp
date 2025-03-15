#ifndef BENCHMARK_HELPER_FUNCTIONS_HPP
#define BENCHMARK_HELPER_FUNCTIONS_HPP

#include <filesystem>
#include <string>

namespace bench::utils {

// Load the first line from a given file. Used for loading single fen from files.
//
// throws std::runtime_error if the file cannot be opened or read
[[nodiscard]] std::string loadFenFromFile(const std::filesystem::path& file_path);

} // namespace bench::utils
#endif // BENCHMARK_HELPER_FUNCTIONS_HPP
