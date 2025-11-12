#ifndef BITCRUSHER_QUEEN_LEGAL_MOVES_HPP
#define BITCRUSHER_QUEEN_LEGAL_MOVES_HPP

#include "attack_generators/diagonal_slider_attacks.hpp"
#include "attack_generators/horizontal_vertical_slider_attacks.hpp"
#include "bitboard_enums.hpp"
#include "concepts.hpp"
#include "restriction_context.hpp"
#include "shared_move_generation.hpp"

namespace bitcrusher {

/// @brief Generates all legal queen moves for the given side, respecting restriction constraints.
/// @tparam Side The Color of the side to move(Color::WHITE or Color::BLACK).
/// @tparam MoveGenerationP Move generation scope policy. See MoveGenerationPolicy for available
/// options.
/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @param board The current board state of the position.
/// @param restriction_context Contains check and pin informations.
/// @param sink The move sink object that will store the generated capture moves.
template <Color                Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateLegalQueenMoves(const BoardState&         board,
                             const RestrictionContext& restriction_context,
                             MoveSinkT&                sink) {

    uint64_t queens_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::QUEEN, Side>());

    uint64_t queens_pinned_only_diagonally = board.getBitboard<PieceType::QUEEN, Side>() &
                                             restriction_context.pinmask_diagonal &
                                             ~restriction_context.pinmask_horizontal_vertical;
    uint64_t queens_pinned_only_horizontally_or_vertically =
        board.getBitboard<PieceType::QUEEN, Side>() &
        restriction_context.pinmask_horizontal_vertical & ~restriction_context.pinmask_diagonal;

    // Queens not pinned.
    generateHorizontalVerticalSlidingPieceMoves<PieceType::QUEEN, Side, MoveGenerationP>(
        queens_not_pinned, board, sink, restriction_context.checkmask);
    generateDiagonalSlidingPieceMoves<PieceType::QUEEN, Side, MoveGenerationP>(
        queens_not_pinned, board, sink, restriction_context.checkmask);

    // Queens pinned diagonally.
    generateDiagonalSlidingPieceMoves<PieceType::QUEEN, Side, MoveGenerationP>(
        queens_pinned_only_diagonally, board, sink,
        restriction_context.checkmask & restriction_context.pinmask_diagonal);

    // Queens pinned horizontally/vertically.
    generateHorizontalVerticalSlidingPieceMoves<PieceType::QUEEN, Side, MoveGenerationP>(
        queens_pinned_only_horizontally_or_vertically, board, sink,
        restriction_context.checkmask & restriction_context.pinmask_horizontal_vertical);
}

} // namespace bitcrusher

#endif // BITCRUSHER_QUEEN_LEGAL_MOVES_HPP