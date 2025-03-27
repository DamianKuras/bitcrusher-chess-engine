#ifndef BITCRUSHER_CONCEPTS_HPP
#define BITCRUSHER_CONCEPTS_HPP

#include "move.hpp"

namespace bitcrusher {

// Concept for move sink
template <typename T>
concept MoveSink = requires(T sink, const Move& move) {
    { sink(move) } -> std::same_as<void>;
};

} // namespace bitcrusher

#endif // BITCRUSHER_CONCEPTS_HPP