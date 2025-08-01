#ifndef BITCRUSHER_MOVE_GENERATION_FROM_BITBOARD_HPP
#define BITCRUSHER_MOVE_GENERATION_FROM_BITBOARD_HPP

#include "concepts.hpp"
#include "move.hpp"

namespace bitcrusher {

// Creates moves given a bitboard and an offset to calculate the source square.
template <MoveType MoveT, PieceType MovedPiece, MoveSink SinkT>
inline void createMovesFromBitboard(SinkT&            sink,
                                    uint64_t          move_to_target_squares,
                                    int               offset_to_create_target_square,
                                    const BoardState& board) {
    if constexpr (MoveT == MoveType::PROMOTION || MoveT == MoveType::PROMOTION_CAPTURE) {
        while (move_to_target_squares != EMPTY_BITBOARD) {
            Square to_square = utils::popFirstSetSquare(move_to_target_squares);

            sink(Move::createMove<MoveT, PieceType::KNIGHT>(
                to_square - offset_to_create_target_square, to_square, board));
            sink(Move::createMove<MoveT, PieceType::BISHOP>(
                to_square - offset_to_create_target_square, to_square, board));
            sink(Move::createMove<MoveT, PieceType::ROOK>(
                to_square - offset_to_create_target_square, to_square, board));
            sink(Move::createMove<MoveT, PieceType::QUEEN>(
                to_square - offset_to_create_target_square, to_square, board));
        }
    } else {
        while (move_to_target_squares != EMPTY_BITBOARD) {
            Square to_square = utils::popFirstSetSquare(move_to_target_squares);
            sink(Move::createMove<MoveT, MovedPiece>(to_square - offset_to_create_target_square,
                                                     to_square, board));
        }
    }
}

// Creates moves given a bitboard and an explicit source square.
template <MoveType MoveT, PieceType MovedPiece, MoveSink Sink>
inline void createMovesFromBitboard(Sink&             sink,
                                    uint64_t          move_to_target_squares,
                                    Square            move_from,
                                    const BoardState& board) {

    if constexpr (MoveT == MoveType::PROMOTION || MoveT == MoveType::PROMOTION_CAPTURE) {
        while (move_to_target_squares != EMPTY_BITBOARD) {
            Square to_square = utils::popFirstSetSquare(move_to_target_squares);

            sink(Move::createMove<MoveT, PieceType::KNIGHT>(move_from, to_square, board));
            sink(Move::createMove<MoveT, PieceType::BISHOP>(move_from, to_square, board));
            sink(Move::createMove<MoveT, PieceType::ROOK>(move_from, to_square, board));
            sink(Move::createMove<MoveT, PieceType::QUEEN>(move_from, to_square, board));
        }

    } else {
        while (move_to_target_squares != EMPTY_BITBOARD) {
            Square to_square = utils::popFirstSetSquare(move_to_target_squares);
            sink(Move::createMove<MoveT, MovedPiece>(move_from, to_square, board));
        }
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_MOVE_GENERATION_FROM_BITBOARD_HPP