#ifndef BITCRUSHER_ROOK_LEGAL_MOVES_HPP
#define BITCRUSHER_ROOK_LEGAL_MOVES_HPP

#include "attack_generators/horizontal_vertical_slider_attacks.hpp"
#include "concepts.hpp"
#include "restriction_context.hpp"
#include "shared_move_generation.hpp"

namespace bitcrusher {

/// @brief Generates all legal rook moves for the given side, respecting restriction constraints.
/// @tparam Side The Color of the side to move(Color::WHITE or Color::BLACK).
/// @tparam MoveGenerationP  Move generation scope policy. See MoveGenerationPolicy for available
/// options.
/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @param board The current board state of the position.
/// @param restriction_context Contains check and pin informations.
/// @param sink The move sink object that will store the generated capture moves.
template <Color                Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateLegalRookMoves(const BoardState&         board,
                            const RestrictionContext& restriction_context,
                            MoveSinkT&                sink) {
    // Rooks not pinned.
    uint64_t rooks_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::ROOK, Side>());
    generateHorizontalVerticalSlidingPieceMoves<PieceType::ROOK, Side, MoveGenerationP>(
        rooks_not_pinned, board, sink, restriction_context.checkmask);

    // Rooks pinned horizontally/vertically.
    uint64_t rooks_pinned_only_hv = board.getBitboard<PieceType::ROOK, Side>() &
                                    restriction_context.pinmask_horizontal_vertical &
                                    ~restriction_context.pinmask_diagonal;
    generateHorizontalVerticalSlidingPieceMoves<PieceType::ROOK, Side, MoveGenerationP>(
        rooks_pinned_only_hv, board, sink,
        restriction_context.checkmask & restriction_context.pinmask_horizontal_vertical);
}

} // namespace bitcrusher

#endif // BITCRUSHER_ROOK_LEGAL_MOVES_HPP