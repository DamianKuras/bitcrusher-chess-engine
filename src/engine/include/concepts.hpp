#ifndef BITCRUSHER_CONCEPTS_HPP
#define BITCRUSHER_CONCEPTS_HPP

#include "bitboard_enums.hpp"
#include "move.hpp"
#include <concepts>
#include <generator>

namespace bitcrusher {

// Helper concept to test if a type is either void or std::generator<Move>
template <typename R>
concept IsVoidOrGenerator = std::same_as<R, void> || std::same_as<R, std::generator<Move>>;

// Concept for move sink
// which is satisfied by any type "T" such that it has () overload accepting
// move
template <typename T>
concept MoveSink = requires(T sink, const Move& move) {
    { sink(move) } -> IsVoidOrGenerator;
};

template <Direction D>
concept Horizontal = (D == Direction::LEFT || D == Direction::RIGHT);

} // namespace bitcrusher

#endif // BITCRUSHER_CONCEPTS_HPP