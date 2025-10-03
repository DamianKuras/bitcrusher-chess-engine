#ifndef BITCRUSHER_KING_LEGAL_MOVES_HPP
#define BITCRUSHER_KING_LEGAL_MOVES_HPP

#include "attack_generators/king_attacks.hpp"
#include "attack_generators/squares_attacked.hpp"
#include "bitboard_enums.hpp"
#include "concepts.hpp"
#include "move.hpp"
#include "restriction_context.hpp"
#include "shared_move_generation.hpp"

namespace bitcrusher {

const uint64_t SQUARES_BETWEEN_WHITE_KINGSIDE_CASTLE_NOT_OCCUPIED_OR_ATTACKED =
    convert::toBitboard(Square::F1, Square::G1);

const uint64_t SQUARES_BETWEEN_BLACK_KINGSIDE_CASTLE_NOT_OCCUPIED_OR_ATTACKED =
    convert::toBitboard(Square::F8, Square::G8);

const uint64_t SQUARES_BETWEEN_WHITE_QUEENSIDE_CASTLE_NOT_OCCUPIED =
    convert::toBitboard(Square::B1, Square::C1, Square::D1);
const uint64_t SQUARES_BETWEEN_BLACK_QUEENSIDE_CASTLE_NOT_OCCUPIED =
    convert::toBitboard(Square::B8, Square::C8, Square::D8);

const uint64_t SQUARES_BETWEEN_WHITE_QUEENSIDE_CASTLE_NOT_ATTACKED =
    convert::toBitboard(Square::C1, Square::D1);
const uint64_t SQUARES_BETWEEN_BLACK_QUEENSIDE_CASTLE_NOT_ATTACKED =
    convert::toBitboard(Square::C8, Square::D8);

template <Color                Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateLegalKingMoves(const BoardState&        board,
                            const RestrictionContext restriction_context,
                            MoveSinkT&               sink) {
    Square king_square = utils::getFirstSetSquare(board.getBitboard<PieceType::KING, Side>());

    // Captures.
    const uint64_t king_attacks = generateKingAttacks(board.getBitboard<PieceType::KING, Side>()) &
                                  (~generateSquaresAttackedXRayingOpponentKing<! Side>(board));
    generateOrderedCaptures<Side, PieceType::KING>(king_attacks, sink, board, king_square);
    if constexpr (MoveGenerationP == MoveGenerationPolicy::CAPTURES_ONLY) {
        return;
    }

    // Non captures and non castling.
    uint64_t king_quiet_moves = king_attacks & board.getEmptySquares();
    createMovesFromBitboard<MoveType::QUIET, PieceType::KING, Side>(sink, king_quiet_moves,
                                                                    king_square);

    // Castling Moves.
    uint64_t enemy_attacked_squares = generateSquaresAttacked<! Side>(board);
    if (restriction_context.check_count == 0) {
        if constexpr (Side == Color::WHITE) {
            if (board.hasCastlingRights<CastlingRights::WHITE_KINGSIDE>() &&
                board.isEmpty(SQUARES_BETWEEN_WHITE_KINGSIDE_CASTLE_NOT_OCCUPIED_OR_ATTACKED) &&
                ! (SQUARES_BETWEEN_WHITE_KINGSIDE_CASTLE_NOT_OCCUPIED_OR_ATTACKED &
                   enemy_attacked_squares)) {
                sink.template emplace<MoveType::KINGSIDE_CASTLE, PieceType::KING, Side>(Square::E1,
                                                                                        Square::G1);
            }

            if (board.hasCastlingRights<CastlingRights::WHITE_QUEENSIDE>() &&
                board.isEmpty(SQUARES_BETWEEN_WHITE_QUEENSIDE_CASTLE_NOT_OCCUPIED) &&
                board.isNotAttackedByOpponent<Side>(
                    SQUARES_BETWEEN_WHITE_QUEENSIDE_CASTLE_NOT_ATTACKED, enemy_attacked_squares)) {
                sink.template emplace<MoveType::QUEENSIDE_CASTLE, PieceType::KING, Side>(
                    Square::E1, Square::C1);
            }
        } else // Side == Color::Black
        {
            if (board.hasCastlingRights<CastlingRights::BLACK_KINGSIDE>() &&
                board.isEmpty(SQUARES_BETWEEN_BLACK_KINGSIDE_CASTLE_NOT_OCCUPIED_OR_ATTACKED) &&
                board.isNotAttackedByOpponent<Side>(
                    SQUARES_BETWEEN_BLACK_KINGSIDE_CASTLE_NOT_OCCUPIED_OR_ATTACKED,
                    enemy_attacked_squares)) {
                sink.template emplace<MoveType::KINGSIDE_CASTLE, PieceType::KING, Side>(Square::E8,
                                                                                        Square::G8);
            }
            if (board.hasCastlingRights<CastlingRights::BLACK_QUEENSIDE>() &&
                board.isEmpty(SQUARES_BETWEEN_BLACK_QUEENSIDE_CASTLE_NOT_OCCUPIED) &&
                board.isNotAttackedByOpponent<Side>(
                    SQUARES_BETWEEN_BLACK_QUEENSIDE_CASTLE_NOT_ATTACKED, enemy_attacked_squares)) {
                sink.template emplace<MoveType::QUEENSIDE_CASTLE, PieceType::KING, Side>(
                    Square::E8, Square::C8);
            };
        }
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_KING_LEGAL_MOVES_HPP