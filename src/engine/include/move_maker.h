#ifndef BITCRUSHER_MOVE_MAKER_H_
#define BITCRUSHER_MOVE_MAKER_H_

#include "bitboard.h"
#include "board_state.h"
#include "move.h"

namespace bitcrusher {

namespace internal {

template <bool IsWhiteMove>
constexpr void updateCastlingRightsOnKingOrRookMove(BoardState& board, const Move& move) {
    if constexpr (IsWhiteMove) {
        if (move.moved_piece == Piece::WHITE_KING) {
            board.removeWhiteCastlingRights();
        } else if (move.moved_piece == Piece::WHITE_ROOK) {
            if (move.from == Square::H1) {
                board.removeWhiteKingsideCastlingRight();
            }
            if (move.from == Square::A1) {
                board.removeWhiteQueensideCastlingRight();
            }
        }
    } else {
        if (move.moved_piece == Piece::BLACK_KING) {
            board.removeBlackCastlingRights();
        } else if (move.moved_piece == Piece::BLACK_ROOK) {
            if (move.from == Square::H8) {
                board.removeBlackKingsideCastlingRight();
            }
            if (move.from == Square::A8) {
                board.removeBlackQueensideCastlingRight();
            }
        }
    }
}

template <bool IsWhiteMove>
constexpr void updateCastlingRightsOnRookCapture(BoardState& board, const Square movedToSquare) {
    if constexpr (IsWhiteMove) {   // Check if White is capturing a Black rook
        if (movedToSquare == Square::H8) {
            board.removeBlackKingsideCastlingRight();
        }
        if (movedToSquare == Square::A8) {
            board.removeBlackQueensideCastlingRight();
        }
    } else { // Check if Black is capturing a White rook
        if (movedToSquare == Square::H1) {
            board.removeWhiteKingsideCastlingRight();
        }
        if (movedToSquare == Square::A1) {
            board.removeWhiteQueensideCastlingRight();
        }
    }
}

template <bool IsWhiteMove>
static constexpr void makeMove(BoardState& board, const Move& move) noexcept {
    // Remove enemy piece if move is en passant or capture
    if (move.isEnPassant()) {
        if constexpr (IsWhiteMove) {
            clearSquare(board.getBitboard<Piece::BLACK_PAWN>(), move.to - FILES_PER_RANK);
        } else {
            clearSquare(board.getBitboard<Piece::WHITE_PAWN>(), move.to + FILES_PER_RANK);
        }
    } else if (move.isCapture()) {
        internal::updateCastlingRightsOnRookCapture<IsWhiteMove>(board, move.to);
        clearSquare(board.getBitboard(move.captured_piece), move.to);
    }

    if (move.isPromotion()) {
        if constexpr (IsWhiteMove) {
            clearSquare(board.getBitboard<Piece::WHITE_PAWN>(), move.from);
        } else {
            clearSquare(board.getBitboard<Piece::BLACK_PAWN>(), move.from);
        }
        setSquare(board.getBitboard(move.moved_piece), move.to);
    } else {
        // Move our piece
        bitcrusher::movePiece(board.getBitboard(move.moved_piece), move.from, move.to);
    }

    // If move was a castling move we need to move rook as well as previously moved king
    if (move.flag == MoveFlag::KINGSIDE_CASTLE) {
        if constexpr (IsWhiteMove) {
            movePiece(board.getBitboard<Piece::WHITE_ROOK>(), Square::H1, Square::F1);
        } else {
            movePiece(board.getBitboard<Piece::BLACK_ROOK>(), Square::H8, Square::F8);
        }
    } else if (move.flag == MoveFlag::QUEENSIDE_CASTLE) {
        if constexpr (IsWhiteMove) {
            movePiece(board.getBitboard<Piece::WHITE_ROOK>(), Square::A1, Square::D1);
        } else {
            movePiece(board.getBitboard<Piece::BLACK_ROOK>(), Square::A8, Square::D8);
        }
    }
    // Update castling rights based on king/rook movement
    internal::updateCastlingRightsOnKingOrRookMove<IsWhiteMove>(board, move);

    // Update en passant square
    board.en_passant_square = (move.flag == MoveFlag::DOUBLE_PAWN_PUSH)
                                  ? enPassantTarget(move.from, move.to)
                                  : Square::NULL_SQUARE;

    // Update move counters
    board.halfmove_clock =
        (move.isPromotion() || move.isCapture() ||
         (move.moved_piece == Piece::WHITE_PAWN || move.moved_piece == Piece::BLACK_PAWN))
            ? 0
            : board.halfmove_clock + 1;
    if constexpr (! IsWhiteMove) {
        board.fullmove_number++;
    }
    // Update side to move;
    board.is_white_move = ! board.is_white_move;
}

} // namespace internal

// Non-templated entry point that dispatches based on side to move.
static constexpr void makeMove(BoardState& board, const Move& move) noexcept {
    board.is_white_move ? internal::makeMove<true>(board, move)
                        : internal::makeMove<false>(board, move);
}

} // namespace bitcrusher
#endif // BITCRUSHER_MOVE_MAKER_H_
