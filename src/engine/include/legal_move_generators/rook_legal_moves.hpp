#ifndef BITCRUSHER_ROOK_LEGAL_MOVES_HPP
#define BITCRUSHER_ROOK_LEGAL_MOVES_HPP

#include "attack_generators/horizontal_vertical_slider_attacks.hpp"
#include "concepts.hpp"
#include "move_generation_from_bitboard.hpp"
#include "restriction_context.hpp"

namespace bitcrusher {

template <MoveSink MoveSinkT, Color Side>
void generateLegalRookMoves(const BoardState&         board,
                            const RestrictionContext& restriction_context,
                            MoveSinkT&                sink) {
    uint64_t rooks_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::ROOK, Side>());
    uint64_t rooks_pinned_only_hv = board.getBitboard<PieceType::ROOK, Side>() &
                                    restriction_context.pinmask_horizontal_vertical &
                                    ~restriction_context.pinmask_diagonal;
    while (rooks_not_pinned != EMPTY_BITBOARD) {
        Square   rook_sq = utils::popFirstSetSquare(rooks_not_pinned);
        uint64_t rook_bb = convert::toBitboard(rook_sq);
        uint64_t rook_attacks =
            generateHorizontalVerticalAttacks(rook_bb, board.getAllOccupancy()) &
            restriction_context.checkmask;
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
            restriction_context.checkmask & restriction_context.pinmask_horizontal_vertical;
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