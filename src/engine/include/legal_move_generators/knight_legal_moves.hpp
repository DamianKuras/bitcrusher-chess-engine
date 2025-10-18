#ifndef BITCRUSHER_KNIGHT_LEGAL_MOVES_HPP
#define BITCRUSHER_KNIGHT_LEGAL_MOVES_HPP

#include "attack_generators/knight_attacks.hpp"
#include "concepts.hpp"
#include "move.hpp"
#include "restriction_context.hpp"
#include "shared_move_generation.hpp"

namespace bitcrusher {

template <Color                Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateLegalKnightMoves(const BoardState&         board,
                              const RestrictionContext& restriction_context,
                              MoveSinkT&                sink) {
    uint64_t knights_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::KNIGHT, Side>());
    while (knights_not_pinned) {
        Square knight_square = utils::popFirstSetSquare(knights_not_pinned);

        uint64_t knight_attacks =
            generateKnightAttacks(knight_square) & restriction_context.checkmask;
        uint64_t knight_quiet_moves = knight_attacks & board.getEmptySquares();
        generateOrderedCaptures<Side, PieceType::KNIGHT>(knight_attacks, sink, board,
                                                         knight_square);
        if constexpr (MoveGenerationP == MoveGenerationPolicy::CAPTURES_ONLY) {
            return;
        }
        createMovesFromBitboard<MoveType::QUIET, PieceType::KNIGHT, Side>(sink, knight_quiet_moves,
                                                                          knight_square);
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_KNIGHT_LEGAL_MOVES_HPP