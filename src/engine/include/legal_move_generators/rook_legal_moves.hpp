#ifndef BITCRUSHER_ROOK_LEGAL_MOVES_HPP
#define BITCRUSHER_ROOK_LEGAL_MOVES_HPP

#include "attack_generators/horizontal_vertical_slider_attacks.hpp"
#include "concepts.hpp"
#include "restriction_context.hpp"
#include "shared_move_generation.hpp"

namespace bitcrusher {

template <Color                Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateLegalRookMoves(const BoardState&         board,
                            const RestrictionContext& restriction_context,
                            MoveSinkT&                sink) {
    uint64_t rooks_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::ROOK, Side>());

    generateSlidingPieceMoves<PieceType::ROOK, Side, generateHorizontalVerticalAttacks,
                              MoveGenerationP>(rooks_not_pinned, board, sink,
                                               restriction_context.checkmask);

    uint64_t rooks_pinned_only_hv = board.getBitboard<PieceType::ROOK, Side>() &
                                    restriction_context.pinmask_horizontal_vertical &
                                    ~restriction_context.pinmask_diagonal;
    generateSlidingPieceMoves<PieceType::ROOK, Side, generateHorizontalVerticalAttacks,
                              MoveGenerationP>(rooks_pinned_only_hv, board, sink,
                                               restriction_context.checkmask &
                                                   restriction_context.pinmask_horizontal_vertical);
}

} // namespace bitcrusher

#endif // BITCRUSHER_ROOK_LEGAL_MOVES_HPP