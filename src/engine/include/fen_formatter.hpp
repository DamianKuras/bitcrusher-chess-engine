#ifndef BITCRUSHER_FEN_FORMATTER_HPP
#define BITCRUSHER_FEN_FORMATTER_HPP

#include "bitboard_utils.hpp"
#include "board_state.hpp"
#include <cstdint>
#include <string_view>
#include <utility>

namespace bitcrusher {

namespace internal {

const uint8_t DECIMAL_BASE = 10;

[[nodiscard]] constexpr bool isDigit(char c) noexcept {
    return static_cast<unsigned>(c - '0') < DECIMAL_BASE;
}

const int CHAR_TO_PIECE_TABLE_SIZE = 115;

consteval std::array<Piece, CHAR_TO_PIECE_TABLE_SIZE>
createChatToPieceLookup() {
    std::array<Piece, CHAR_TO_PIECE_TABLE_SIZE> table{};
    table['P'] = Piece::WHITE_PAWN;
    table['N'] = Piece::WHITE_KNIGHT;
    table['B'] = Piece::WHITE_BISHOP;
    table['R'] = Piece::WHITE_ROOK;
    table['Q'] = Piece::WHITE_QUEEN;
    table['K'] = Piece::WHITE_KING;
    table['p'] = Piece::BLACK_PAWN;
    table['n'] = Piece::BLACK_KNIGHT;
    table['b'] = Piece::BLACK_BISHOP;
    table['r'] = Piece::BLACK_ROOK;
    table['q'] = Piece::BLACK_QUEEN;
    table['k'] = Piece::BLACK_KING;
    return table;
}

constexpr Piece getCapturedPiece(const BoardState &board, Square square) {
    if (board.isWhiteMove()) {
        for (const auto piece : BLACK_PIECES) {
            if (utils::isSquareSet(board.getBitboard(piece), square)) {
                return piece;
            }
        }
    } else {
        for (const auto piece : WHITE_PIECES) {
            if (utils::isSquareSet(board.getBitboard(piece), square)) {
                return piece;
            }
        }
    }
    std::unreachable();
}

constexpr Piece getOurPiece(const BoardState &board, Square square) {
    if (board.isWhiteMove()) {
        for (const auto piece : WHITE_PIECES) {
            if (utils::isSquareSet(board.getBitboard(piece), square)) {
                return piece;
            }
        }
    } else {
        for (const auto piece : BLACK_PIECES) {
            if (utils::isSquareSet(board.getBitboard(piece), square)) {
                return piece;
            }
        }
    }
    std::unreachable();
}

constexpr std::array<Piece, CHAR_TO_PIECE_TABLE_SIZE> CHAR_TO_PIECE =
    createChatToPieceLookup();
} // namespace internal

// Parses the FEN string into BoardState
// Assumes the FEN input is correct and contains at
// least board, side to move, en passant square and side to move
static inline void parseFEN(std::string_view fen, BoardState &boardState) {
    boardState.reset();
    auto iterator = fen.begin();
    Square square = Square::A8; // Starting square for FEN board description

    // Parse board piece placement
    while (*iterator != ' ') {
        const char fen_character = *iterator;
        if (internal::isDigit(fen_character)) {
            // Advance the square by the the number of empty squares
            square += convert::toDigit(fen_character);
        } else if (fen_character != '/') {
            boardState.setPieceOnSquare(internal::CHAR_TO_PIECE[fen_character],
                                        square);
            square += 1;
        }
        ++iterator;
    }

    // Parse side to move
    ++iterator; // Skip space
    boardState.setSideToMove(*iterator == 'w' ? Color::WHITE : Color::BLACK);

    // Parse castling rights
    iterator += 2; // Skip side to move and space
    while (*iterator != ' ') {
        if (*iterator == 'K') {
            boardState.addWhiteKingsideCastlingRight();
        } else if (*iterator == 'Q') {
            boardState.addWhiteQueensideCastlingRight();
        } else if (*iterator == 'k') {
            boardState.addBlackKingsideCastlingRight();
        } else if (*iterator == 'q') {
            boardState.addBlackQueensideCastlingRight();
        }
        ++iterator;
    }

    // Parse en passant square
    ++iterator; // Skip space
    if (*iterator == '-') {
        ++iterator;
    } else {
        File ep_file = convert::toFile(*iterator);
        ++iterator;
        Rank ep_rank = convert::toRank(*iterator);
        boardState.setEnPassantSquare(convert::toSquare(ep_file, ep_rank));
        ++iterator; // skip en passant square
    }

    // If halfmove clock and fullmove number are not specified, return
    if (iterator == fen.end()) {
        return;
    }
    // Parse halfmove clock
    ++iterator; // Skip space
    uint8_t halfmove_clock = 0;
    while (*iterator != ' ') {
        halfmove_clock = halfmove_clock * internal::DECIMAL_BASE +
                         convert::toDigit(*iterator);
        ++iterator;
    }
    boardState.setHalfmoveClock(halfmove_clock);
    // Parse fullmove number
    ++iterator; // Skip space
    uint16_t fullmove_number = 0;
    while (iterator != fen.end()) {
        fullmove_number = fullmove_number * internal::DECIMAL_BASE +
                          convert::toDigit(*iterator);
        ++iterator;
    }
    boardState.setFullmoveNumber(fullmove_number);
}

} // namespace bitcrusher

#endif // BITCRUSHER_FEN_FORMATTER_HPP
