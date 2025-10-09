#ifndef BITCRUSHER_UCI_CONSTANTS_HPP
#define BITCRUSHER_UCI_CONSTANTS_HPP

#define __cpp_lib_constexpr_format 2025XXL
#include <format>
#include <string>
#include <string_view>

namespace bitcrusher {

inline constexpr std::string_view UCI_ID_STRING{"id name Bitcrusher Chess Engine 0.1 \n"
                                                "id author Damian Kuraś \n"};

struct UciSpinOption {
    std::string name;
    int         default_value{};
    int         min_value{};
    int         max_value{};

    [[nodiscard]] std::string toString() const {

        return std::format("option name {} type spin default {} min {} max {}\n", name,
                           default_value, min_value, max_value);
    }
};

inline constexpr UciSpinOption THREADS{
    .name = "Threads", .default_value = 1, .min_value = 1, .max_value = 1024};

// The value for memory of hash table in MB.
inline constexpr UciSpinOption HASH{
    .name = "Hash", .default_value = 32, .min_value = 1, .max_value = 1024};

inline std::string OPTIONS = THREADS.toString() + HASH.toString();
} // namespace bitcrusher

const int MILLISECONDS_PER_SECONDS = 1000;
#endif // BITCRUSHER_UCI_CONSTANTS_HPP