#ifndef BITCRUSHER_QUEEN_LEGAL_MOVES_HPP
#define BITCRUSHER_QUEEN_LEGAL_MOVES_HPP

#include "attack_generators/diagonal_slider_attacks.hpp"
#include "attack_generators/horizontal_vertical_slider_attacks.hpp"
#include "bitboard_enums.hpp"
#include "concepts.hpp"
#include "restriction_context.hpp"
#include "shared_move_generation.hpp"

namespace bitcrusher {

template <Color                Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateLegalQueenMoves(const BoardState&         board,
                             const RestrictionContext& restriction_context,
                             MoveSinkT&                sink) {

    uint64_t queens_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::QUEEN, Side>());

    generateSlidingPieceMoves<PieceType::QUEEN, Side, generateHorizontalVerticalAttacks,
                              MoveGenerationP>(queens_not_pinned, board, sink,
                                               restriction_context.checkmask);
    generateSlidingPieceMoves<PieceType::QUEEN, Side, generateDiagonalAttacks, MoveGenerationP>(
        queens_not_pinned, board, sink, restriction_context.checkmask);

    uint64_t queens_pinned_only_diagonally = board.getBitboard<PieceType::QUEEN, Side>() &
                                             restriction_context.pinmask_diagonal &
                                             ~restriction_context.pinmask_horizontal_vertical;
    generateSlidingPieceMoves<PieceType::QUEEN, Side, generateDiagonalAttacks, MoveGenerationP>(
        queens_pinned_only_diagonally, board, sink,
        restriction_context.checkmask & restriction_context.pinmask_diagonal);

    uint64_t queens_pinned_only_horizontally_or_vertically =
        board.getBitboard<PieceType::QUEEN, Side>() &
        restriction_context.pinmask_horizontal_vertical & ~restriction_context.pinmask_diagonal;
    generateSlidingPieceMoves<PieceType::QUEEN, Side, generateHorizontalVerticalAttacks,
                              MoveGenerationP>(
        queens_pinned_only_horizontally_or_vertically, board, sink,
        restriction_context.checkmask & restriction_context.pinmask_horizontal_vertical);
}

} // namespace bitcrusher

#endif // BITCRUSHER_QUEEN_LEGAL_MOVES_HPP