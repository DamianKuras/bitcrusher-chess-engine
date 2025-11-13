#ifndef BITCRUSHER_EVALUATION_HPP
#define BITCRUSHER_EVALUATION_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include <array>
#include <bit>
#include <unordered_map>
#include <utility>

namespace bitcrusher {

// Piece-square tables
namespace internal {
// clang-format off
inline constexpr std::array<int, 64> middle_game_pawn_table{{
      0,   0,   0,   0,   0,   0,  0,   0,
     98, 134,  61,  95,  68, 126, 34, -11,
     -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
      0,   0,   0,   0,   0,   0,  0,   0,
}};

inline constexpr std::array<int, 64> end_game_pawn_table{{
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0,
}};

inline constexpr std::array<int, 64> middle_game_knight_table{{
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
}};

inline constexpr std::array<int, 64> end_game_knight_table{{
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
}};

inline constexpr std::array<int, 64> middle_game_bishop_table{{
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
}};

inline constexpr std::array<int, 64> end_game_bishop_table{{
    -14, -21, -11,  -8, -7,  -9, -17, -24,
     -8,  -4,   7, -12, -3, -13,  -4, -14,
      2,  -8,   0,  -1, -2,   6,   0,   4,
     -3,   9,  12,   9, 14,  10,   3,   2,
     -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,
}};

inline constexpr std::array<int, 64> middle_game_rook_table{{
     32,  42,  32,  51, 63,  9,  31,  43,
     27,  32,  58,  62, 80, 67,  26,  44,
     -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,
}};

inline constexpr std::array<int, 64> end_game_rook_table{{
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
     7,  7,  7,  5,  4,  -3,  -5,  -3,
     4,  3, 13,  1,  2,   1,  -1,   2,
     3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
}};

inline constexpr std::array<int, 64> middle_game_queen_table{{
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
}};

inline constexpr std::array<int, 64> end_game_queen_table{{
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
}};

inline constexpr std::array<int, 64> middle_game_king_table{{
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
}};

inline constexpr std::array<int, 64> end_game_king_table{{
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
}};
// clang-format on
inline constexpr int MAX_GAME_PHASE = 24;

consteval auto buildMiddleGameBaseValues() noexcept {
    EnumIndexedArray<int, PieceType, 6> values;
    values[PieceType::PAWN]   = 82;
    values[PieceType::KNIGHT] = 337;
    values[PieceType::BISHOP] = 365;
    values[PieceType::ROOK]   = 477;
    values[PieceType::QUEEN]  = 1025;
    values[PieceType::KING]   = 0;
    return values;
}

consteval auto buildEndGameBaseValues() noexcept {
    EnumIndexedArray<int, PieceType, 6> values;
    values[PieceType::PAWN]   = 94;
    values[PieceType::KNIGHT] = 281;
    values[PieceType::BISHOP] = 297;
    values[PieceType::ROOK]   = 512;
    values[PieceType::QUEEN]  = 936;
    values[PieceType::KING]   = 0;
    return values;
}

consteval auto buildGamePhaseInc() noexcept {
    EnumIndexedArray<int, Piece, PIECE_COUNT> phase;
    phase[Piece::WHITE_PAWN]   = 0;
    phase[Piece::WHITE_KNIGHT] = 1;
    phase[Piece::WHITE_BISHOP] = 1;
    phase[Piece::WHITE_ROOK]   = 2;
    phase[Piece::WHITE_QUEEN]  = 4;
    phase[Piece::WHITE_KING]   = 0;
    phase[Piece::BLACK_PAWN]   = 0;
    phase[Piece::BLACK_KNIGHT] = 1;
    phase[Piece::BLACK_BISHOP] = 1;
    phase[Piece::BLACK_ROOK]   = 2;
    phase[Piece::BLACK_QUEEN]  = 4;
    phase[Piece::BLACK_KING]   = 0;
    return phase;
}

struct PhaseData {
    EnumIndexedArray<int, PieceType, PIECE_COUNT_PER_SIDE>       base_values;
    std::array<const std::array<int, 64>*, PIECE_COUNT_PER_SIDE> piece_square_tables;
};

inline constexpr PhaseData MiddleGame{.base_values         = buildMiddleGameBaseValues(),
                                      .piece_square_tables = {
                                          &middle_game_pawn_table,
                                          &middle_game_knight_table,
                                          &middle_game_bishop_table,
                                          &middle_game_rook_table,
                                          &middle_game_queen_table,
                                          &middle_game_king_table,
                                      }};
inline constexpr PhaseData EndGame{.base_values         = buildEndGameBaseValues(),
                                   .piece_square_tables = {
                                       &end_game_pawn_table,
                                       &end_game_knight_table,
                                       &end_game_bishop_table,
                                       &end_game_rook_table,
                                       &end_game_queen_table,
                                       &end_game_king_table,
                                   }};

enum class EvaluationPhase { MIDDLE_GAME, END_GAME };

consteval auto buildPhaseSpecificPieceSquareTable(const PhaseData& phase) {
    EnumIndexedArray<EnumIndexedArray<int, Square, SQUARE_COUNT>, Piece, PIECE_COUNT> table;
    auto fill = [&](Piece white, Piece black, PieceType type,
                    const std::array<int, 64>& piece_square_table) {
        int base_phase_value = phase.base_values[type];
        for (int square_index = 0; square_index < SQUARE_COUNT; ++square_index) {
            table[white][static_cast<Square>(square_index)] =
                base_phase_value + piece_square_table[square_index];
            table[black][static_cast<Square>(square_index)] =
                base_phase_value + piece_square_table[square_index ^ 56];
        }
    };

    fill(Piece::WHITE_PAWN, Piece::BLACK_PAWN, PieceType::PAWN, *phase.piece_square_tables[0]);
    fill(Piece::WHITE_KNIGHT, Piece::BLACK_KNIGHT, PieceType::KNIGHT,
         *phase.piece_square_tables[1]);
    fill(Piece::WHITE_BISHOP, Piece::BLACK_BISHOP, PieceType::BISHOP,
         *phase.piece_square_tables[2]);
    fill(Piece::WHITE_ROOK, Piece::BLACK_ROOK, PieceType::ROOK, *phase.piece_square_tables[3]);
    fill(Piece::WHITE_QUEEN, Piece::BLACK_QUEEN, PieceType::QUEEN, *phase.piece_square_tables[4]);
    fill(Piece::WHITE_KING, Piece::BLACK_KING, PieceType::KING, *phase.piece_square_tables[5]);

    return table;
}

inline constinit auto game_phase_inc = buildGamePhaseInc();
inline constinit auto mg_table       = buildPhaseSpecificPieceSquareTable(MiddleGame);
inline constinit auto eg_table       = buildPhaseSpecificPieceSquareTable(EndGame);
} // namespace internal

/// @brief Returns evaluation relative to side to move.
/// @param board The current board state of the evaluated position.
/// @param side  The Color of the side evaluation is relative for(Color::WHITE or Color::BLACK).
/// @return Centipawn evaluation of the position.
[[nodiscard]] inline int basicEval(const BoardState& board, Color side) noexcept {
    int middle_game_white = 0, middle_game_black = 0;
    int end_game_white = 0, end_game_black = 0;
    int game_phase = 0;

    // Evaluate white pieces.
    for (const auto piece : WHITE_PIECES) {
        uint64_t bb = board.getBitboard(piece);
        while (bb) {
            const Square sq = utils::popFirstSetSquare(bb);
            middle_game_white += internal::mg_table[piece][sq];
            end_game_white += internal::eg_table[piece][sq];
            game_phase += internal::game_phase_inc[piece];
        }
    }

    // Evaluate black pieces.
    for (const auto piece : BLACK_PIECES) {
        uint64_t bb = board.getBitboard(piece);
        while (bb) {
            const Square sq = utils::popFirstSetSquare(bb);
            middle_game_black += internal::mg_table[piece][sq];
            end_game_black += internal::eg_table[piece][sq];
            game_phase += internal::game_phase_inc[piece];
        }
    }

    const int mg_score = (middle_game_white - middle_game_black);
    const int eg_score = (end_game_white - end_game_black);

    // Tapered evaluation.
    game_phase         = std::min(game_phase, 24);
    const int eg_phase = 24 - game_phase;
    int       eval     = (mg_score * game_phase + eg_score * eg_phase) / 24;
    return (side == Color::WHITE) ? eval : -eval;
}

} // namespace bitcrusher

#endif // BITCRUSHER_EVALUATION_HPP