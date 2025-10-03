#ifndef BITCRUSHER_BISHOP_LEGAL_MOVES_HPP
#define BITCRUSHER_BISHOP_LEGAL_MOVES_HPP

#include "attack_generators/diagonal_slider_attacks.hpp"
#include "bitboard_enums.hpp"
#include "concepts.hpp"
#include "restriction_context.hpp"
#include "shared_move_generation.hpp"
#include <cstdint>

namespace bitcrusher {

template <Color                Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateLegalBishopMoves(const BoardState&        board,
                              const RestrictionContext restriction_context,
                              MoveSinkT&               sink) {
    uint64_t bishops_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::BISHOP, Side>());
    generateSlidingPieceMoves<PieceType::BISHOP, Side, generateDiagonalAttacks, MoveGenerationP>(bishops_not_pinned, board, sink,
                                                       restriction_context.checkmask);

    uint64_t bishops_pinned_only_diagonally =
        board.getBitboard<PieceType::BISHOP, Side>() & restriction_context.pinmask_diagonal;
    generateSlidingPieceMoves<PieceType::BISHOP, Side, generateDiagonalAttacks, MoveGenerationP>(bishops_pinned_only_diagonally, board, sink,
                                                       restriction_context.checkmask &
                                                           restriction_context.pinmask_diagonal);
}

} // namespace bitcrusher

#endif // BITCRUSHER_BISHOP_LEGAL_MOVES_HPP