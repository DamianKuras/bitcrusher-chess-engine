#ifndef BITCRUSHER_QUEEN_LEGAL_MOVES_HPP
#define BITCRUSHER_QUEEN_LEGAL_MOVES_HPP

#include "bitboard_concepts.hpp"
#include "restriction_context.hpp"

namespace bitcrusher {

template <MoveSink MoveSinkT, Color Side>
void generateLegalQueenMoves(const BoardState&         board,
                             const RestrictionContext& restriction_context,
                             MoveSinkT&                sink) {

    uint64_t queens_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::QUEEN, Side>());

    uint64_t queens_pinned_only_diagonally = board.getBitboard<PieceType::QUEEN, Side>() &
                                             restriction_context.pinmask_diagonal &
                                             ~restriction_context.pinmask_horizontal_vertical;
    while (queens_not_pinned != EMPTY_BITBOARD) {
        Square   queen_sq      = utils::popFirstSetSquare(queens_not_pinned);
        uint64_t queen_bb      = convert::toBitboard(queen_sq);
        uint64_t queen_attacks = generateDiagonalAttacks(queen_bb, board.getAllOccupancy()) &
                                 restriction_context.checkmask;
        uint64_t queen_captures    = queen_attacks & board.getOpponentOccupancy<Side>();
        uint64_t queen_quiet_moves = queen_attacks & board.getEmptySquares();
        createMovesFromBitboard<MoveType::CAPTURE, PieceType::QUEEN>(sink, queen_captures, queen_sq,
                                                                     board);
        createMovesFromBitboard<MoveType::QUIET, PieceType::QUEEN>(sink, queen_quiet_moves,
                                                                   queen_sq, board);
    }
    while (queens_pinned_only_diagonally != EMPTY_BITBOARD) {
        Square   queen_sq      = utils::popFirstSetSquare(queens_pinned_only_diagonally);
        uint64_t queen_bb      = convert::toBitboard(queen_sq);
        uint64_t queen_attacks = generateDiagonalAttacks(queen_bb, board.getAllOccupancy()) &
                                 restriction_context.checkmask &
                                 restriction_context.pinmask_diagonal;

        uint64_t queen_captures    = queen_attacks & board.getOpponentOccupancy<Side>();
        uint64_t queen_quiet_moves = queen_attacks & board.getEmptySquares();

        createMovesFromBitboard<MoveType::CAPTURE, PieceType::QUEEN>(sink, queen_captures, queen_sq,
                                                                     board);
        createMovesFromBitboard<MoveType::QUIET, PieceType::QUEEN>(sink, queen_quiet_moves,
                                                                   queen_sq, board);
    }

    queens_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::QUEEN, Side>());
    uint64_t queens_pinned_only_horizontally = board.getBitboard<PieceType::QUEEN, Side>() &
                                               restriction_context.pinmask_horizontal_vertical &
                                               ~restriction_context.pinmask_diagonal;
    while (queens_not_pinned != EMPTY_BITBOARD) {
        Square   queen_sq = utils::popFirstSetSquare(queens_not_pinned);
        uint64_t queen_bb = convert::toBitboard(queen_sq);
        uint64_t queen_attacks =
            generateHorizontalVerticalAttacks(queen_bb, board.getAllOccupancy()) &
            restriction_context.checkmask;
        uint64_t queen_captures    = queen_attacks & board.getOpponentOccupancy<Side>();
        uint64_t queen_quiet_moves = queen_attacks & board.getEmptySquares();
        createMovesFromBitboard<MoveType::CAPTURE, PieceType::QUEEN>(sink, queen_captures, queen_sq,
                                                                     board);
        createMovesFromBitboard<MoveType::QUIET, PieceType::QUEEN>(sink, queen_quiet_moves,
                                                                   queen_sq, board);
    }
    while (queens_pinned_only_horizontally != EMPTY_BITBOARD) {
        Square   rook_sq = utils::popFirstSetSquare(queens_pinned_only_horizontally);
        uint64_t rook_bb = convert::toBitboard(rook_sq);
        uint64_t rook_attacks =
            generateHorizontalVerticalAttacks(rook_bb, board.getAllOccupancy()) &
            restriction_context.checkmask & restriction_context.pinmask_horizontal_vertical;
        uint64_t rook_captures    = rook_attacks & board.getOpponentOccupancy<Side>();
        uint64_t rook_quiet_moves = rook_attacks & board.getEmptySquares();
        createMovesFromBitboard<MoveType::CAPTURE, PieceType::QUEEN>(sink, rook_captures, rook_sq,
                                                                     board);
        createMovesFromBitboard<MoveType::QUIET, PieceType::QUEEN>(sink, rook_quiet_moves, rook_sq,
                                                                   board);
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_QUEEN_LEGAL_MOVES_HPP