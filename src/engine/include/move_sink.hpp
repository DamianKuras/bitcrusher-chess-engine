#ifndef BITCRUSHER_MOVE_SINK_HPP
#define BITCRUSHER_MOVE_SINK_HPP

// #include "concepts.hpp"
#include "bitboard_enums.hpp"
#include "concepts.hpp"
#include "move.hpp"
#include <cassert>

namespace bitcrusher {

// https://lichess.org/@/Tobs40/blog/why-a-reachable-position-can-have-at-most-218-playable-moves/a5xdxeqs
const int MAX_LEGAL_MOVES = 218;
const int MAX_PLY         = 90;

struct FastMoveSink : MoveSinkBase<FastMoveSink> {
    std::array<std::array<Move, MAX_LEGAL_MOVES>, MAX_PLY> moves{};
    std::array<int, MAX_PLY>                               count{};
    int                                                    ply = 0;

    void clearPly(int ply) noexcept { count[ply] = 0; }

    template <MoveType  MoveT,
              PieceType MovedOrPromotedToPiece,
              Color     SideToMove,
              PieceType CapturedPiece = PieceType::NONE>
    void emplace(Square from, Square to) noexcept {
        assert(count[ply] < MAX_LEGAL_MOVES);
        moves[ply][count[ply]++].setMove<MoveT, MovedOrPromotedToPiece, SideToMove, CapturedPiece>(
            from, to);
    }
};

static_assert(MoveSink<FastMoveSink>);

} // namespace bitcrusher

#endif // BITCRUSHER_MOVE_SINK_HPP