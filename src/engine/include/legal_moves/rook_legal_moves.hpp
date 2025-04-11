#ifndef BITCRUSHER_ROOK_LEGAL_MOVES_HPP
#define BITCRUSHER_ROOK_LEGAL_MOVES_HPP

#include "concepts.hpp"

namespace bitcrusher {

template <MoveSink MoveSinkT, Color Side>
void generateLegalRookMoves(const BoardState& board, MoveSinkT& sink) {
    uint64_t rooks_not_pinned = board.getNonRestrictedBitboard<PieceType::ROOK, Side>();
    uint64_t rooks_pinned_only_hv = board.getBitboard<PieceType::ROOK, Side>() &
                                             board.restriction_context.pinmask_horizontal_vertical &
                                             ~board.restriction_context.pinmask_diagonal;
    while (rooks_not_pinned != EMPTY_BITBOARD) {
        Square   rook_sq = utils::popFirstSetSquare(rooks_not_pinned);
        uint64_t rook_bb = convert::toBitboard(rook_sq);
        uint64_t rook_attacks =
            generateHorizontalVerticalAttacks(rook_bb, board.getAllOccupancy()) &
            board.restriction_context.checkmask;
        uint64_t rook_captures    = rook_attacks & board.getOpponentOccupancy<Side>();
        uint64_t rook_quiet_moves = rook_attacks & board.getEmptySquares();
        createMovesFromBitboard<MoveType::CAPTURE, PieceType::ROOK>(sink, rook_captures, rook_sq,
                                                                    board);
        createMovesFromBitboard<MoveType::QUIET, PieceType::ROOK>(sink, rook_quiet_moves, rook_sq,
                                                                  board);
    }
    while (rooks_pinned_only_hv != EMPTY_BITBOARD) {
        Square   rook_sq = utils::popFirstSetSquare(rooks_pinned_only_hv);
        uint64_t rook_bb = convert::toBitboard(rook_sq);
        uint64_t rook_attacks =
            generateHorizontalVerticalAttacks(rook_bb, board.getAllOccupancy()) &
            board.restriction_context.checkmask &
            board.restriction_context.pinmask_horizontal_vertical;
        uint64_t rook_captures    = rook_attacks & board.getOpponentOccupancy<Side>();
        uint64_t rook_quiet_moves = rook_attacks & board.getEmptySquares();
        createMovesFromBitboard<MoveType::CAPTURE, PieceType::ROOK>(sink, rook_captures, rook_sq,
                                                                    board);
        createMovesFromBitboard<MoveType::QUIET, PieceType::ROOK>(sink, rook_quiet_moves, rook_sq,
                                                                  board);
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_ROOK_LEGAL_MOVES_HPP