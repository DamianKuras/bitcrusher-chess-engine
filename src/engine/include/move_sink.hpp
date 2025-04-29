#ifndef BITCRUSHER_MOVE_SINK_HPP
#define BITCRUSHER_MOVE_SINK_HPP

#include "bitboard_concepts.hpp"
#include "move.hpp"

namespace bitcrusher {

constexpr std::size_t MAX_LEGAL_MOVES = 256;

struct FastMoveSink {
    std::array<Move, MAX_LEGAL_MOVES> moves;
    std::size_t                       count = 0;

    void clear() noexcept { count = 0; }

    void operator()(const Move& move) noexcept { moves[count++] = move; }
};

static_assert(MoveSink<FastMoveSink>);

} // namespace bitcrusher

#endif // BITCRUSHER_MOVE_SINK_HPP