#ifndef BITCRUSHER_FEN_FORMATTER_H_
#define BITCRUSHER_FEN_FORMATTER_H_

#include "board_state.h"
#include <string_view>

namespace bitcrusher {

namespace internal {
constexpr bitcrusher::Piece charToPiece(const char c) {
    switch (c) {
    case 'P':
        return bitcrusher::Piece::WHITE_PAWNS;
    case 'N':
        return bitcrusher::Piece::WHITE_KNIGHTS;
    case 'B':
        return bitcrusher::Piece::WHITE_BISHOPS;
    case 'R':
        return bitcrusher::Piece::WHITE_ROOKS;
    case 'Q':
        return bitcrusher::Piece::WHITE_QUEENS;
    case 'K':
        return bitcrusher::Piece::WHITE_KING;
    case 'p':
        return bitcrusher::Piece::BLACK_PAWNS;
    case 'n':
        return bitcrusher::Piece::BLACK_KNIGHTS;
    case 'b':
        return bitcrusher::Piece::BLACK_BISHOPS;
    case 'r':
        return bitcrusher::Piece::BLACK_ROOKS;
    case 'q':
        return bitcrusher::Piece::BLACK_QUEENS;
    case 'k':
        return bitcrusher::Piece::BLACK_KING;
    default:
        return bitcrusher::Piece::WHITE_PAWNS; // Fallback
    }
}

constexpr bitcrusher::CastlingRight charToCastlingRight(const char c) {
    switch (c) {
    case 'K':
        return CastlingRight::WHITE_KINGSIDE;
        break;
    case 'Q':
        return CastlingRight::WHITE_QUEENSIDE;
        break;
    case 'k':
        return CastlingRight::BLACK_KINGSIDE;
        break;
    case 'q':
        return CastlingRight::BLACK_QUEENSIDE;
        break;
    default:
        return CastlingRight::NONE;
    }
}

} // namespace internal

// Parses the FEN string into BoardState.
// This function does not perform validation and assumes the FEN input is correct.
constexpr void parseFEN(std::string_view fen, bitcrusher::BoardState& boardState) {
    // Reset
    for (auto& bitboard : boardState.bitboards) {
        bitboard = 0ULL;
    }
    boardState.castling_rights   = CastlingRight::NONE;
    boardState.en_passant_square = Square::NULL_SQUARE;
    boardState.fullmove_number   = 0;
    boardState.halfmove_clock    = 0;
    boardState.is_white_move     = true;

    int rank = 7; // Start from rank 8 (0-indexed)
    int file = 0;
    int i    = 0;

    // Parse the board position
    while (fen[i] != ' ') {
        if (fen[i] == '/') // Move to the next rank and reset file
        {
            rank--;
            file = 0;
        } else if (fen[i] >= '0' && fen[i] <= '9') // Skip the number of empty squares
        {
            file += fen[i] - '0';
        } else {
            uint8_t square_index = (rank * 8) + file; // Calculate the square index (0-63)
            Piece   piece_type   = internal::charToPiece(fen[i]);
            bitcrusher::setSquare(boardState[piece_type], static_cast<Square>(square_index));
            ++file;
        }
        ++i;
    }

    // Parse side to move
    ++i; // Skip space
    boardState.is_white_move = (fen[i] == 'w');

    // Parse castling rights
    i += 2; // Skip side to move and space
    while (fen[i] != '\0' && fen[i] != ' ') {
        boardState.castling_rights |= internal::charToCastlingRight(fen[i]);
        ++i;
    }

    // Parse en passant square
    ++i; // Skip space
    if (fen[i] != '-') {
        int ep_file                  = fen[i] - 'a';
        int ep_rank                  = fen[i + 1] - '1';
        boardState.en_passant_square = static_cast<Square>((ep_rank * 8) + ep_file);
        i += 2; // skip en passant square
    } else {
        i += 1;
    }

    // Parse halfmove clock
    ++i; // Skip space
    while (fen[i] != '\0' && fen[i] != ' ') {
        boardState.halfmove_clock = boardState.halfmove_clock * 10 + (fen[i] - '0');
        ++i;
    }

    // Parse fullmove number
    ++i; // Skip space
    while (fen[i] != '\0' && fen[i] != ' ') {
        boardState.fullmove_number = boardState.fullmove_number * 10 + (fen[i] - '0');
        ++i;
    }
}

} // namespace bitcrusher
#endif // BITCRUSHER_FEN_FORMATTER_H_
