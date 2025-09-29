#ifndef BITCRUSHER_BITBOARD_ENUMS_HPP
#define BITCRUSHER_BITBOARD_ENUMS_HPP

#include <cstdint>
#include <ostream>
#include <utility>

namespace bitcrusher {

inline constexpr int BOARD_DIMENSION{8};

inline constexpr int PIECE_COUNT = 12;

inline constexpr int CASTLING_RIGHTS_COUNT = 4;

inline constexpr std::uint8_t PIECE_COUNT_PER_SIDE = 6;

inline constexpr std::string_view INITIAL_POSITION_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

inline constexpr std::uint64_t EMPTY_BITBOARD{0};

inline constexpr std::uint64_t FULL_BITBOARD{~EMPTY_BITBOARD};

// Represents chess board squares using 0-based indexing from A8 (0) to H1 (63).
//
// Null square is equal to 64 outside of the 0-63 board range.
// clang-format off
enum class Square : std::uint8_t {
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1,
    NULL_SQUARE,
};

const int SQUARE_COUNT{64};

enum class Rank: std::uint8_t {
    R_8,
    R_7,
    R_6,
    R_5,
    R_4,
    R_3,
    R_2,    
    R_1,
};

enum class File: std::uint8_t {
    A, B, C, D, E, F, G, H,
};
// clang-format on

enum class Color : bool { WHITE, BLACK };
enum class Side : bool { KINGSIDE, QUEENSIDE };

enum class Diagonal : std::uint8_t {
    H8,
    G8H7,
    F8H6,
    E8H5,
    D8H4,
    C8H3,
    B8H2,
    A8H1, // longest diagonal
    A7G1,
    A6F1,
    A5E1,
    A4D1,
    A3C1,
    A2B1,
    A1,
};

const int DIAGONAL_COUNT = 15;

enum class CounterDiagonal : uint8_t {
    A8,
    A7B8,
    A6C8,
    A5D8,
    A4E8,
    A3F8,
    A2G8,
    A1H8, // longest counter diagonal
    B1H7,
    C1H6,
    D1H5,
    E1H4,
    F1H3,
    G1H2,
    H1
};

enum class Direction : std::uint8_t {
    TOP,
    LEFT,
    RIGHT,
    BOTTOM,
};

enum class PieceType : std::uint8_t { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE };

enum class Piece : std::uint8_t {
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,

    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,
    COUNT,
    NONE,
};

enum class SlidingPieceType : std::uint8_t { DIAGONAL, HORIZONTAL_VERTICAL };

// Square operators
[[nodiscard]] static constexpr Square operator+(Square square, int offset) noexcept {
    return static_cast<Square>(std::to_underlying(square) + offset);
}

static constexpr void operator+=(Square& square, int offset) noexcept {
    square = square + offset;
}

[[nodiscard]] static constexpr Square operator-(Square square, int offset) noexcept {
    return static_cast<Square>(std::to_underlying(square) - offset);
}

static constexpr Square operator-=(Square square, int offset) noexcept {
    return square - offset;
}

[[nodiscard]] static constexpr Color operator!(Color color) noexcept {
    return static_cast<Color>(! std::to_underlying(color));
}

struct SquareChars {
    char file = 'a';
    char rank = '1';
};

inline std::ostream& operator<<(std::ostream& os, const SquareChars& sq) {
    os << sq.file << sq.rank;
    return os;
}

// Each of the castling right has its own bit.
enum class CastlingRights : std::uint8_t {
    NONE                  = 0,
    WHITE_KINGSIDE        = 1 << 0,                                        // 0001
    WHITE_QUEENSIDE       = 1 << 1,                                        // 0010
    WHITE_CASTLING_RIGHTS = WHITE_KINGSIDE | WHITE_QUEENSIDE,              // 0011
    BLACK_KINGSIDE        = 1 << 2,                                        // 0100
    BLACK_QUEENSIDE       = 1 << 3,                                        // 1000
    BLACK_CASTLING_RIGHTS = BLACK_KINGSIDE | BLACK_QUEENSIDE,              // 1100
    ALL_CASTLING_RIGHTS   = WHITE_CASTLING_RIGHTS | BLACK_CASTLING_RIGHTS, // 1111
};

// Concepts to constrain enum usage
template <CastlingRights Right>
concept SingularCastlingRight =
    Right == CastlingRights::WHITE_KINGSIDE || Right == CastlingRights::WHITE_QUEENSIDE ||
    Right == CastlingRights::BLACK_KINGSIDE || Right == CastlingRights::BLACK_QUEENSIDE;

constexpr CastlingRights operator|(CastlingRights lhs, CastlingRights rhs) noexcept {
    return static_cast<CastlingRights>(static_cast<std::uint8_t>(lhs) |
                                       static_cast<std::uint8_t>(rhs));
}

constexpr CastlingRights& operator|=(CastlingRights& lhs, CastlingRights rhs) noexcept {
    lhs = lhs | rhs;
    return lhs;
}

constexpr CastlingRights operator&(CastlingRights lhs, CastlingRights rhs) noexcept {
    return static_cast<CastlingRights>(static_cast<std::uint8_t>(lhs) &
                                       static_cast<std::uint8_t>(rhs));
}

constexpr CastlingRights& operator&=(CastlingRights& lhs, CastlingRights rhs) noexcept {
    lhs = lhs & rhs;
    return lhs;
}

constexpr CastlingRights operator~(CastlingRights rhs) noexcept {
    return static_cast<CastlingRights>(~static_cast<uint8_t>(rhs));
}

// usage:
// enum class MyEnum { A, B, C, Count };
// EnumIndexedArray<int, MyEnum, static_cast<std::size_t>(MyEnum::Count)> arr;
// arr[MyEnum::B] = 42;
template <typename T, typename EnumType, std::size_t Size> class EnumIndexedArray {
    std::array<T, Size> data_{};

public:
    // Non-const access
    constexpr T& operator[](EnumType e) noexcept { return data_[static_cast<std::size_t>(e)]; }

    // Const access
    constexpr const T& operator[](EnumType e) const noexcept {
        return data_[static_cast<std::size_t>(e)];
    }

    // Fill method
    constexpr void fill(const T& value) noexcept {
        for (std::size_t i = 0; i < Size; ++i) {
            data_[i] = value;
        }
    }

    constexpr const std::array<T, Size>& data() const noexcept { return data_; }

    constexpr std::array<T, Size>& data() noexcept { return data_; }

    EnumIndexedArray() = default;

    bool operator==(const EnumIndexedArray& rhs) const = default;
};

} // namespace bitcrusher

#endif // BITCRUSHER_BITBOARD_ENUMS_HPP