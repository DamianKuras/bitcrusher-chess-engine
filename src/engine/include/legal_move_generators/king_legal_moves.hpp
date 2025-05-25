#ifndef BITCRUSHER_KING_LEGAL_MOVES_HPP
#define BITCRUSHER_KING_LEGAL_MOVES_HPP

#include "attack_generators/king_attacks.hpp"
#include "attack_generators/squares_attacked.hpp"
#include "bitboard_concepts.hpp"
#include "move_generation_from_bitboard.hpp"
#include "restriction_context.hpp"

namespace bitcrusher {

template <MoveSink MoveSinkT, Color Side>
void generateLegalKingMoves(const BoardState&        board,
                            const RestrictionContext restriction_context,
                            MoveSinkT&               sink) {
    const uint64_t king_moves = generateKingAttacks(board.getBitboard<PieceType::KING, Side>()) &
                                (~generateSquaresAttackedXRayingOpponentKing<! Side>(board));

    uint64_t king_captures    = king_moves & board.getOpponentOccupancy<Side>();
    uint64_t king_quiet_moves = king_moves & board.getEmptySquares();
    Square   our_king_square = utils::getFirstSetSquare(board.getBitboard<PieceType::KING, Side>());

    createMovesFromBitboard<MoveType::CAPTURE, PieceType::KING>(sink, king_captures,
                                                                our_king_square, board);
    createMovesFromBitboard<MoveType::QUIET, PieceType::KING>(sink, king_quiet_moves,
                                                              our_king_square, board);
    uint64_t squares_between_white_kingside_castle_not_occupied_or_attacked =
        convert::toBitboard(Square::F1, Square::G1);

    uint64_t squares_between_black_kingside_castle_not_occupied_or_attacked =
        convert::toBitboard(Square::F8, Square::G8);

    uint64_t squares_between_white_queenside_castle_not_occupied =
        convert::toBitboard(Square::B1, Square::C1, Square::D1);
    uint64_t squares_between_black_queenside_castle_not_occupied =
        convert::toBitboard(Square::B8, Square::C8, Square::D8);

    uint64_t squares_between_white_queenside_castle_not_attacked =
        convert::toBitboard(Square::C1, Square::D1);
    uint64_t squares_between_black_queenside_castle_not_attacked =
        convert::toBitboard(Square::C8, Square::D8);

    uint64_t enemy_attacked_squares = generateSquaresAttacked<! Side>(board);

    if (restriction_context.check_count == 0) {
        if constexpr (Side == Color::WHITE) {
            if (board.hasWhiteKingsideCastlingRight() &&
                board.isEmpty(squares_between_white_kingside_castle_not_occupied_or_attacked) &&
                ! (squares_between_white_kingside_castle_not_occupied_or_attacked &
                   enemy_attacked_squares)) {
                sink(Move::createCastlingMove<Color::WHITE, Side::KINGSIDE>());
            }

            if (board.hasWhiteQueensideCastlingRight() &&
                board.isEmpty(squares_between_white_queenside_castle_not_occupied) &&
                board.isNotAttackedByOpponent<Side>(
                    squares_between_white_queenside_castle_not_attacked, enemy_attacked_squares)) {

                sink(Move::createCastlingMove<Color::WHITE, Side::QUEENSIDE>());
            }
        } else // Side == Color::Black
        {
            if (board.hasBlackKingsideCastlingRight() &&
                board.isEmpty(squares_between_black_kingside_castle_not_occupied_or_attacked) &&
                board.isNotAttackedByOpponent<Side>(
                    squares_between_black_kingside_castle_not_occupied_or_attacked,
                    enemy_attacked_squares)) {
                sink(Move::createCastlingMove<Side, Side::KINGSIDE>());
            }
            if (board.hasBlackQueensideCastlingRight() &&
                board.isEmpty(squares_between_black_queenside_castle_not_occupied) &&
                board.isNotAttackedByOpponent<Side>(
                    squares_between_black_queenside_castle_not_attacked, enemy_attacked_squares)) {
                sink(Move::createCastlingMove<Side, Side::QUEENSIDE>());
            };
        }
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_KING_LEGAL_MOVES_HPP