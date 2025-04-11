#ifndef BITCRUSHER_BISHOP_LEGAL_MOVES_HPP
#define BITCRUSHER_BISHOP_LEGAL_MOVES_HPP

#include "attacks/diagonal_slider_attacks.hpp"
#include "bitboard_conversions.hpp"
#include "concepts.hpp"
#include "move_generation_from_bitboard.hpp"


namespace bitcrusher {

template <MoveSink MoveSinkT, Color Side>
void generateLegalBishopMoves(const BoardState& board, MoveSinkT& sink) {
    uint64_t bishops_not_pinned = board.getNonRestrictedBitboard<PieceType::BISHOP, Side>();
    uint64_t bishops_pinned_only_diagonally =
        board.getBitboard<PieceType::BISHOP, Side>() & board.restriction_context.pinmask_diagonal;
    while (bishops_not_pinned != EMPTY_BITBOARD) {
        Square   bishop_sq      = utils::popFirstSetSquare(bishops_not_pinned);
        uint64_t bishop_bb      = convert::toBitboard(bishop_sq);
        uint64_t bishop_attacks = generateDiagonalAttacks(bishop_bb, board.getAllOccupancy()) &
                                  board.restriction_context.checkmask;
        uint64_t bishop_captures    = bishop_attacks & board.getOpponentOccupancy<Side>();
        uint64_t bishop_quiet_moves = bishop_attacks & board.getEmptySquares();
        createMovesFromBitboard<MoveType::CAPTURE, PieceType::BISHOP>(sink, bishop_captures,
                                                                      bishop_sq, board);
        createMovesFromBitboard<MoveType::QUIET, PieceType::BISHOP>(sink, bishop_quiet_moves,
                                                                    bishop_sq, board);
    }
    while (bishops_pinned_only_diagonally != EMPTY_BITBOARD) {
        Square   bishop_sq      = utils::popFirstSetSquare(bishops_pinned_only_diagonally);
        uint64_t bishop_bb      = convert::toBitboard(bishop_sq);
        uint64_t bishop_attacks = generateDiagonalAttacks(bishop_bb, board.getAllOccupancy()) &
                                  board.restriction_context.checkmask &
                                  board.restriction_context.pinmask_diagonal;
        uint64_t bishop_captures    = bishop_attacks & board.getOpponentOccupancy<Side>();
        uint64_t bishop_quiet_moves = bishop_attacks & board.getEmptySquares();
        createMovesFromBitboard<MoveType::CAPTURE, PieceType::BISHOP>(sink, bishop_captures,
                                                                      bishop_sq, board);
        createMovesFromBitboard<MoveType::QUIET, PieceType::BISHOP>(sink, bishop_quiet_moves,
                                                                    bishop_sq, board);
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_BISHOP_LEGAL_MOVES_HPP