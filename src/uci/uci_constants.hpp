#ifndef BITCRUSHER_UCI_CONSTANTS_HPP
#define BITCRUSHER_UCI_CONSTANTS_HPP

#include <cstdint>
#include <string>
#include <string_view>

namespace bitcrusher {

const std::string_view UCI_ID_STRING{"id name Bitcrusher Chess Engine 0.1 \n"
                                     "id author Damian Kuraś \n"};

struct UciSpinOption {
    std::string name;
    int         default_value;
    int         min_value;
    int         max_value;

    [[nodiscard]] std::string toString() const {
        return "option name " + name + " type spin default " + std::to_string(default_value) +
               " min " + std::to_string(min_value) + " max " + std::to_string(max_value) + "\n";
    }
};

const UciSpinOption THREADS{
    .name = "Threads", .default_value = 1, .min_value = 1, .max_value = 1024};

// the value in MB for memory for hash tables can be changed,
const UciSpinOption HASH{.name = "Hash", .default_value = 32, .min_value = 1, .max_value = 1024};

const std::string OPTIONS = THREADS.toString() + HASH.toString();
} // namespace bitcrusher

#endif // BITCRUSHER_UCI_CONSTANTS_HPP