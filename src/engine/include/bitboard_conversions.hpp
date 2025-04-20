#ifndef BITCRUSHER_BITBOARD_CONVERSIONS_HPP
#define BITCRUSHER_BITBOARD_CONVERSIONS_HPP

#include "bitboard_enums.hpp"
#include <array>
#include <cstdint>
#include <utility>

namespace bitcrusher::convert {

[[nodiscard]] constexpr int toDigit(const char c) noexcept {
    return c - '0';
}

[[nodiscard]] constexpr Square toSquare(File file, Rank rank) noexcept {
    return static_cast<Square>((std::to_underlying(rank) * BOARD_DIMENSION) +
                               std::to_underlying(file));
}

[[nodiscard]] constexpr File toFile(char c) noexcept {
    return static_cast<File>(c - 'a');
}

[[nodiscard]] constexpr Rank toRank(char c) noexcept {
    return static_cast<Rank>(BOARD_DIMENSION - convert::toDigit(c));
}

// Convert a single Square to its bitboard representation.
[[nodiscard]]
static constexpr uint64_t toBitboard(Square square) noexcept {
    return 1ULL << std::to_underlying(square);
}

// Variadic template to create a bitboard from multiple squares.
//
// Example:
//
//  uint64_t initalWhiteRookBitboard =
//  toBitboard(Square::A1,Square::H1)
//
// or
//
// uint64_t file_A = toBitboard(Square::A1, Square::A2, Square::A3,
// Square::A4, Square::A5, Square::A6, Square::A7, Square::A8);
template <typename... Squares>
    requires(std::same_as<Squares, Square> && ...)
[[nodiscard]] consteval uint64_t toBitboard(Squares... squares) {
    uint64_t bitboard{0ULL};
    for (Square square : {squares...}) {
        bitboard |= convert::toBitboard(square);
    }
    return bitboard;
}

constexpr Rank toRank(Square square) {
    return static_cast<Rank>((std::to_underlying(square) / BOARD_DIMENSION));
}

constexpr File toFile(Square square) {
    return static_cast<File>((std::to_underlying(square) % BOARD_DIMENSION));
}

template <Color Side> constexpr Color toOppositeColor() {
    if constexpr (Side == Color::WHITE) {
        return Color::BLACK;
    } else { // Side == Color::BLACK
        return Color::WHITE;
    }
}

// Return offset to get the square at the position in the choosen direction
[[nodiscard]] constexpr int toDelta(Direction direction) noexcept {
    switch (direction) {
    case Direction::TOP:
        return -static_cast<int8_t>(BOARD_DIMENSION);
    case Direction::BOTTOM:
        return static_cast<int8_t>(BOARD_DIMENSION);
    case Direction::LEFT:
        return -1;
    case Direction::RIGHT:
        return 1;
    default:
        std::unreachable();
    }
}

// clang-format off
constexpr std::array<Diagonal, 64> SQUARE_TO_DIAGONAL = {
    // rank 8 (a8-h8)
    Diagonal::A8H1, Diagonal::B8H2, Diagonal::C8H3, Diagonal::D8H4, Diagonal::E8H5, Diagonal::F8H6, Diagonal::G8H7, Diagonal::H8,
    // rank 7 (a7-h7)
    Diagonal::A7G1, Diagonal::A8H1, Diagonal::B8H2, Diagonal::C8H3, Diagonal::D8H4, Diagonal::E8H5, Diagonal::F8H6, Diagonal::G8H7,
    // rank 6 (a6-h6)
    Diagonal::A6F1, Diagonal::A7G1, Diagonal::A8H1, Diagonal::B8H2, Diagonal::C8H3, Diagonal::D8H4, Diagonal::E8H5, Diagonal::F8H6,
    // rank 5 (a5-h5)
    Diagonal::A5E1, Diagonal::A6F1, Diagonal::A7G1, Diagonal::A8H1, Diagonal::B8H2, Diagonal::C8H3, Diagonal::D8H4, Diagonal::E8H5,
    // rank 4 (a4-h4)
    Diagonal::A4D1, Diagonal::A5E1, Diagonal::A6F1, Diagonal::A7G1, Diagonal::A8H1, Diagonal::B8H2, Diagonal::C8H3, Diagonal::D8H4,
    // rank 3 (a3-h3)
    Diagonal::A3C1, Diagonal::A4D1, Diagonal::A5E1, Diagonal::A6F1, Diagonal::A7G1, Diagonal::A8H1, Diagonal::B8H2, Diagonal::C8H3,
    // rank 2 (a2-h2)
    Diagonal::A2B1, Diagonal::A3C1, Diagonal::A4D1, Diagonal::A5E1, Diagonal::A6F1, Diagonal::A7G1, Diagonal::A8H1, Diagonal::B8H2,
    // rank 1 (a1-h1)
    Diagonal::A1,    Diagonal::A2B1, Diagonal::A3C1, Diagonal::A4D1, Diagonal::A5E1, Diagonal::A6F1, Diagonal::A7G1, Diagonal::A8H1
};

// Lookup table mapping each square (bitboard index 0=a1,...,63=h8) to its CounterDiagonal enum
constexpr std::array<CounterDiagonal, 64> SQUARE_TO_COUNTER_DIAGONAL = {{
    // rank 8 (a8-h8)
    CounterDiagonal::A8,   CounterDiagonal::A7B8, CounterDiagonal::A6C8, CounterDiagonal::A5D8, CounterDiagonal::A4E8, CounterDiagonal::A3F8, CounterDiagonal::A2G8, CounterDiagonal::A1H8,
    // rank 7 (a7-h7)
    CounterDiagonal::A7B8, CounterDiagonal::A6C8, CounterDiagonal::A5D8, CounterDiagonal::A4E8, CounterDiagonal::A3F8, CounterDiagonal::A2G8, CounterDiagonal::A1H8, CounterDiagonal::B1H7,
    // rank 6 (a6-h6)
    CounterDiagonal::A6C8, CounterDiagonal::A5D8, CounterDiagonal::A4E8, CounterDiagonal::A3F8, CounterDiagonal::A2G8, CounterDiagonal::A1H8, CounterDiagonal::B1H7, CounterDiagonal::C1H6,
    // rank 5 (a5-h5)
    CounterDiagonal::A5D8, CounterDiagonal::A4E8, CounterDiagonal::A3F8, CounterDiagonal::A2G8, CounterDiagonal::A1H8, CounterDiagonal::B1H7, CounterDiagonal::C1H6, CounterDiagonal::D1H5,
    // rank 4 (a4-h4)
    CounterDiagonal::A4E8, CounterDiagonal::A3F8, CounterDiagonal::A2G8, CounterDiagonal::A1H8, CounterDiagonal::B1H7, CounterDiagonal::C1H6, CounterDiagonal::D1H5, CounterDiagonal::E1H4,
    // rank 3 (a3-h3)
    CounterDiagonal::A3F8, CounterDiagonal::A2G8, CounterDiagonal::A1H8, CounterDiagonal::B1H7, CounterDiagonal::C1H6, CounterDiagonal::D1H5, CounterDiagonal::E1H4, CounterDiagonal::F1H3,
    // rank 2 (a2-h2)
    CounterDiagonal::A2G8, CounterDiagonal::A1H8, CounterDiagonal::B1H7, CounterDiagonal::C1H6, CounterDiagonal::D1H5, CounterDiagonal::E1H4, CounterDiagonal::F1H3, CounterDiagonal::G1H2,
    // rank 1 (a1-h1)
    CounterDiagonal::A1H8, CounterDiagonal::B1H7, CounterDiagonal::C1H6, CounterDiagonal::D1H5, CounterDiagonal::E1H4, CounterDiagonal::F1H3, CounterDiagonal::G1H2, CounterDiagonal::H1
}};


[[nodiscard]] constexpr Diagonal toDiagonal(Square square) noexcept {
    return SQUARE_TO_DIAGONAL[std::to_underlying(square)];
}

[[nodiscard]] constexpr CounterDiagonal toCounterDiagonal(Square square) noexcept {
    return SQUARE_TO_COUNTER_DIAGONAL[std::to_underlying(square)];
}


[[nodiscard]] constexpr char toChar(Rank rank) noexcept {
    if (rank == Rank::R_1) {
        return '1';
    }
    if (rank == Rank::R_2) {
        return '2';
    }
    if (rank == Rank::R_3) {
        return '3';
    }
    if (rank == Rank::R_4) {
        return '4';
    }
    if (rank == Rank::R_5) {
        return '5';
    }
    if (rank == Rank::R_6) {
        return '6';
    }
    if (rank == Rank::R_7) {
        return '7';
    }
    if (rank == Rank::R_8) {
        return '8';
    }
    std::unreachable();
}

[[nodiscard]] constexpr char toChar(File file) noexcept {
    if (file == File::A) {
        return 'a';
    }
    if (file == File::B) {
        return 'b';
    }
    if (file == File::C) {
        return 'c';
    }
    if (file == File::D) {
        return 'd';
    }
    if (file == File::E) {
        return 'e';
    }
    if (file == File::F) {
        return 'f';
    }
    if (file == File::G) {
        return 'g';
    }
    if (file == File::H) {
        return 'h';
    }
    std::unreachable();
}

[[nodiscard]] constexpr char toChar(Piece piece) noexcept {
    switch (piece) {
    case Piece::WHITE_PAWN:
        return 'P';
    case Piece::BLACK_PAWN:
        return 'p';
    case Piece::WHITE_KNIGHT:
        return 'N';
    case Piece::BLACK_KNIGHT:
        return 'n';
    case Piece::WHITE_BISHOP:
        return 'B';
    case Piece::BLACK_BISHOP:
        return 'b';
    case Piece::WHITE_ROOK:
        return 'R';
    case Piece::BLACK_ROOK:
        return 'r';
    case Piece::WHITE_QUEEN:
        return 'Q';
    case Piece::BLACK_QUEEN:
        return 'q';
    case Piece::WHITE_KING:
        return 'K';
    case Piece::BLACK_KING:
        return 'k';
    case Piece::NONE:
        return '.';
    default:
        std::unreachable();
    }
}

[[nodiscard]] constexpr char toPromotionUci(PieceType piece_type) {
    switch (piece_type) {
    case PieceType::KNIGHT:
        return 'n';
    case PieceType::BISHOP:
        return 'b';
    case PieceType::ROOK:
        return 'r';
    case PieceType::QUEEN:
        return 'q';
    default:
        std::unreachable();
    }
}

[[nodiscard]] constexpr PieceType toPromotionPieceType(char piece_char) {
    switch (piece_char) {
    case 'p':
        return PieceType::PAWN;
    case 'n':
        return PieceType::KNIGHT;
    case 'b':
        return PieceType::BISHOP;
    case 'r':
        return PieceType::ROOK;
    case 'q':
        return PieceType::QUEEN;
    default:
        std::unreachable();
    }
}

[[nodiscard]] constexpr SquareChars toChars(Square sq) noexcept {
    return {.file = convert::toChar(convert::toFile(sq)),
            .rank = convert::toChar(convert::toRank(sq))};
}

template <Color C, PieceType PieceT> [[nodiscard]] constexpr Piece toPiece() noexcept {
    if constexpr (C == Color::WHITE) {
        if constexpr (PieceT == PieceType::PAWN) {
            return Piece::WHITE_PAWN;
        } else if constexpr (PieceT == PieceType::KNIGHT) {
            return Piece::WHITE_KNIGHT;
        } else if constexpr (PieceT == PieceType::BISHOP) {
            return Piece::WHITE_BISHOP;
        } else if constexpr (PieceT == PieceType::ROOK) {
            return Piece::WHITE_ROOK;
        } else if constexpr (PieceT == PieceType::QUEEN) {
            return Piece::WHITE_QUEEN;
        } else if constexpr (PieceT == PieceType::KING) {
            return Piece::WHITE_KING;
        }
    } else { // Color::BLACK
        if constexpr (PieceT == PieceType::PAWN) {
            return Piece::BLACK_PAWN;
        } else if constexpr (PieceT == PieceType::KNIGHT) {
            return Piece::BLACK_KNIGHT;
        } else if constexpr (PieceT == PieceType::BISHOP) {
            return Piece::BLACK_BISHOP;
        } else if constexpr (PieceT == PieceType::ROOK) {
            return Piece::BLACK_ROOK;
        } else if constexpr (PieceT == PieceType::QUEEN) {
            return Piece::BLACK_QUEEN;
        } else if constexpr (PieceT == PieceType::KING) {
            return Piece::BLACK_KING;
        }
    }
}

template <Color C> [[nodiscard]] constexpr Piece toPiece(PieceType piece_t) noexcept {
    return static_cast<Piece>(std::to_underlying(piece_t) +
                              (PIECE_COUNT_PER_SIDE * (C == Color::BLACK)));
}

} // namespace bitcrusher::convert

#endif