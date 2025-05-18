#ifndef BITCRUSHER_FEN_FORMATTER_HPP
#define BITCRUSHER_FEN_FORMATTER_HPP

#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"
#include "bitboard_utils.hpp"
#include "board_state.hpp"
#include "move.hpp"
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

consteval std::array<Piece, CHAR_TO_PIECE_TABLE_SIZE> createChatToPieceLookup() {
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

constexpr Piece getCapturedPiece(const BoardState& board, Square square) {
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

constexpr Piece getOurPiece(const BoardState& board, Square square) {
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

constexpr std::array<Piece, CHAR_TO_PIECE_TABLE_SIZE> CHAR_TO_PIECE = createChatToPieceLookup();

} // namespace internal

// Parses the FEN string into BoardState
// Assumes the FEN input is correct and contains at
// least board, side to move, en passant square and side to move
static constexpr void parseFEN(std::string_view fen, BoardState& board) {
    board.reset();
    auto   iterator = fen.begin();
    Square square   = Square::A8; // Starting square for FEN board description

    // Parse board piece placement
    while (*iterator != ' ') {
        const char fen_character = *iterator;
        if (internal::isDigit(fen_character)) {
            // Advance the square by the the number of empty squares
            square += convert::toDigit(fen_character);
        } else if (fen_character != '/') {
            board.addPieceToSquare(internal::CHAR_TO_PIECE[fen_character], square);
            square += 1;
        }
        ++iterator;
    }
    board.calculateOccupancies();

    // Parse side to move
    ++iterator; // Skip space
    board.setSideToMove(*iterator == 'w' ? Color::WHITE : Color::BLACK);

    // Parse castling rights
    iterator += 2; // Skip side to move and space
    while (*iterator != ' ') {
        if (*iterator == 'K') {
            board.addWhiteKingsideCastlingRight();
        } else if (*iterator == 'Q') {
            board.addWhiteQueensideCastlingRight();
        } else if (*iterator == 'k') {
            board.addBlackKingsideCastlingRight();
        } else if (*iterator == 'q') {
            board.addBlackQueensideCastlingRight();
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
        board.setEnPassantSquare(convert::toSquare(ep_file, ep_rank));
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
        halfmove_clock = halfmove_clock * internal::DECIMAL_BASE + convert::toDigit(*iterator);
        ++iterator;
    }
    board.setHalfmoveClock(halfmove_clock);
    // Parse fullmove number
    ++iterator; // Skip space
    uint16_t fullmove_number = 0;
    while (iterator != fen.end()) {
        fullmove_number = fullmove_number * internal::DECIMAL_BASE + convert::toDigit(*iterator);
        ++iterator;
    }
    board.setFullmoveNumber(fullmove_number);
}

static inline Move moveFromUci(std::string_view uci_string, const BoardState& board) {
    Square from = convert::toSquare(convert::toFile(uci_string[0]), convert::toRank(uci_string[1]));
    Square to   = convert::toSquare(convert::toFile(uci_string[2]), convert::toRank(uci_string[3]));
    const uint64_t enemy_occupancy =
        board.isWhiteMove() ? board.generateBlackOccupancy() : board.generateWhiteOccupancy();
    const bool is_non_ep_capture = utils::isSquareSet(enemy_occupancy, to);
    const bool is_promotion      = uci_string.size() == 5;
    const bool is_en_passant     = board.getEnPassantSquare() == to;
    if (is_promotion) {
        PieceType pawn_promoted_to = convert::toPromotionPieceType(uci_string[4]);
        if (is_non_ep_capture) {
            return Move::createPromotionCaptureMove(from, to, pawn_promoted_to,
                                                    board.getPieceTypeOnSquare(to));
        }
        return Move::createPromotionMove(from, to, pawn_promoted_to);
    }

    if (is_non_ep_capture) {
        return Move::createCaptureMove(from, to, board.getPieceTypeOnSquare(from),
                                       board.getPieceTypeOnSquare(to));
    }
    if (is_en_passant) {
        return Move::createEnPassantMove(from, to);
    }
    if (from == Square::E1 && to == Square::G1 && board.hasWhiteKingsideCastlingRight()) {
        return Move::createCastlingMove<Color::WHITE, Side::KINGSIDE>();
    }
    if (from == Square::E1 && to == Square::C1 && board.hasWhiteQueensideCastlingRight()) {
        return Move::createCastlingMove<Color::WHITE, Side::QUEENSIDE>();
    }
    if (from == Square::E8 && to == Square::G8 && board.hasWhiteKingsideCastlingRight()) {
        return Move::createCastlingMove<Color::WHITE, Side::KINGSIDE>();
    }
    if (from == Square::E8 && to == Square::C8 && board.hasBlackQueensideCastlingRight()) {
        return Move::createCastlingMove<Color::WHITE, Side::QUEENSIDE>();
    }
    if (from + offset::calculateOffset<Direction::TOP, Direction::TOP>() == to) {
        return Move::createDoublePawnPushMove(from, to);
    }
    return Move::createQuietMove(from, to, board.getPieceTypeOnSquare(from));
}

} // namespace bitcrusher

#endif // BITCRUSHER_FEN_FORMATTER_HPP
