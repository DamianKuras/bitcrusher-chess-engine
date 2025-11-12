#ifndef BITCRUSHER_BISHOP_LEGAL_MOVES_HPP
#define BITCRUSHER_BISHOP_LEGAL_MOVES_HPP

#include "attack_generators/diagonal_slider_attacks.hpp"
#include "bitboard_enums.hpp"
#include "concepts.hpp"
#include "restriction_context.hpp"
#include "shared_move_generation.hpp"
#include <cstdint>

namespace bitcrusher {

/// @brief Generates all legal bishop moves for the given side, respecting restriction constraints.
///
/// @tparam Side The Color of the side to move(Color::WHITE or Color::BLACK).
/// @tparam MoveGenerationP Move generation scope policy. See MoveGenerationPolicy for available
/// options.
/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @param board  The current board state of the position.
/// @param restriction_context Contains check and pin informations.
/// @param sink The move sink object that will store the generated capture moves.
template <Color                Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateLegalBishopMoves(const BoardState&        board,
                              const RestrictionContext restriction_context,
                              MoveSinkT&               sink) {
    uint64_t bishops_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::BISHOP, Side>());
    generateDiagonalSlidingPieceMoves<PieceType::BISHOP, Side, MoveGenerationP>(
        bishops_not_pinned, board, sink, restriction_context.checkmask);

    uint64_t bishops_pinned_only_diagonally =
        board.getBitboard<PieceType::BISHOP, Side>() & restriction_context.pinmask_diagonal;
    generateDiagonalSlidingPieceMoves<PieceType::BISHOP, Side, MoveGenerationP>(
        bishops_pinned_only_diagonally, board, sink,
        restriction_context.checkmask & restriction_context.pinmask_diagonal);
}

} // namespace bitcrusher

#endif // BITCRUSHER_BISHOP_LEGAL_MOVES_HPP