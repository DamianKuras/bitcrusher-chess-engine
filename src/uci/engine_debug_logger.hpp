#ifndef BITCRUSHER_LOGGER_HPP
#define BITCRUSHER_LOGGER_HPP

#include <string_view>
#ifdef DEBUG
#    include <fstream>

namespace bitcrusher {
const std::string    LOG_FILE_PATH = "./debug_logs/engine_log.txt";
inline std::ofstream log_file(LOG_FILE_PATH, std::ios::app);

inline void logToFile(std::string_view direction, std::string_view message) {
    log_file << direction << ": " << message << '\n';
    log_file.flush();
}

} // namespace bitcrusher
#endif // DEBUG

#endif // BITCRUSHER_LOGGER_HPP