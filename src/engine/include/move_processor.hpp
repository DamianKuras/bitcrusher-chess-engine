#ifndef BITCRUSHER_MOVE_PROCESSOR_HPP
#define BITCRUSHER_MOVE_PROCESSOR_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "move.hpp"
#include <array>

namespace bitcrusher {

namespace internal {

const int MAX_DEPTH = 20;

struct MoveUndo {
    CastlingRights prev_castling_rights{};
    Square         prev_en_passant_square{};
    uint16_t       prev_halfmove_clock{};
    uint16_t       prev_fullmove_number{};
    Color          moving_side{};

    bool operator==(const MoveUndo& rhs) const = default;
};

template <Color SideToMove>
constexpr void updateCastlingRightsOnKingOrRookMove(BoardState& board, const Move& move) {
    if constexpr (SideToMove == Color::WHITE) {
        if (move.movingPiece() == PieceType::KING) {
            board.removeWhiteCastlingRights();
        } else if (move.movingPiece() == PieceType::ROOK) {
            if (move.fromSquare() == Square::H1) {
                board.removeWhiteKingsideCastlingRight();
            } else if (move.fromSquare() == Square::A1) {
                board.removeWhiteQueensideCastlingRight();
            }
        }
    } else if constexpr (SideToMove == Color::BLACK) {
        if (move.movingPiece() == PieceType::KING) {
            board.removeBlackCastlingRights();
        } else if (move.movingPiece() == PieceType::ROOK) {
            if (move.fromSquare() == Square::H8) {
                board.removeBlackKingsideCastlingRight();
            } else if (move.fromSquare() == Square::A8) {
                board.removeBlackQueensideCastlingRight();
            }
        }
    }
}

template <Color SideToMove>
constexpr void updateCastlingRightsOnRookCapture(BoardState& board, const Square to_square) {
    if constexpr (SideToMove == Color::WHITE) { // Check if White is capturing a Black rook
        if (to_square == Square::H8) {
            board.removeBlackKingsideCastlingRight();
        } else if (to_square == Square::A8) {
            board.removeBlackQueensideCastlingRight();
        }
    } else { // Check if Black is capturing a White rook
        if (to_square == Square::H1) {
            board.removeWhiteKingsideCastlingRight();
        } else if (to_square == Square::A1) {
            board.removeWhiteQueensideCastlingRight();
        }
    }
}

template <Color SideToMove>
static constexpr void applyMove(BoardState& board, const Move& move) noexcept {
    // For en passant or capture remove enemy piece and updates castling rights
    if (move.isEnPassant()) {
        board.removePieceFromSquare<! SideToMove>(
            PieceType::PAWN,
            move.toSquare() + ((SideToMove == Color::WHITE) ? BOARD_DIMENSION : -BOARD_DIMENSION));
    } else if (move.isCapture()) {
        internal::updateCastlingRightsOnRookCapture<SideToMove>(board, move.toSquare());
        board.removePieceFromSquare<! SideToMove>(move.capturedPiece(), move.toSquare());
    }

    // Move or promote piece
    if (move.isPromotion()) {
        board.removePieceFromSquare<SideToMove>(PieceType::PAWN, move.fromSquare());
        board.addPieceToSquare<SideToMove>(move.promotionPiece(), move.toSquare());
    } else {
        board.movePiece<SideToMove>(move.movingPiece(), move.fromSquare(), move.toSquare());
    }

    // If move was a castling move we need to move rook as well as king
    if (move.isKingsideCastle()) {
        if constexpr (SideToMove == Color::WHITE) {
            board.movePiece<Color::WHITE>(PieceType::ROOK, Square::H1, Square::F1);
        } else { // SideToMove == Color::BLACK
            board.movePiece<Color::BLACK>(PieceType::ROOK, Square::H8, Square::F8);
        }
    } else if (move.isQueensideCastle()) {
        if constexpr (SideToMove == Color::WHITE) {
            board.movePiece<Color::WHITE>(PieceType::ROOK, Square::A1, Square::D1);

        } else { // SideToMove == Color::BLACK
            board.movePiece<Color::BLACK>(PieceType::ROOK, Square::A8, Square::D8);
        }
    }
    // Update castling rights if move king or rook
    internal::updateCastlingRightsOnKingOrRookMove<SideToMove>(board, move);

    // Update ep square
    if constexpr (SideToMove == Color::WHITE) {
        board.setEnPassantSquare((move.isPawnDoublePush()) ? move.fromSquare() - BOARD_DIMENSION
                                                           : Square::NULL_SQUARE);
    } else {
        board.setEnPassantSquare((move.isPawnDoublePush()) ? move.fromSquare() + BOARD_DIMENSION
                                                           : Square::NULL_SQUARE);
    }

    // Update move counters
    if (move.isPromotion() || move.isCapture() || (move.movingPiece() == PieceType::PAWN)) {
        board.resetHalfmoveClock();
    } else {
        board.incrementHalfmoveClock();
    }

    if constexpr (SideToMove == Color::BLACK) {
        board.incrementFullmoveNumber();
    }
    board.calculateOccupancies();
    board.toggleSideToMove();
}

template <Color SideToMove>
constexpr void
undoMove(BoardState& board, const Move& move, const internal::MoveUndo& undo) noexcept {
    // Restore enemy piece if move is en passant or capture
    if (move.isEnPassant()) {
        board.addPieceToSquare<! SideToMove>(
            PieceType::PAWN,
            move.toSquare() + ((SideToMove == Color::WHITE) ? BOARD_DIMENSION : -BOARD_DIMENSION));
    } else if (move.isCapture()) {
        board.addPieceToSquare<! SideToMove>(move.capturedPiece(), move.toSquare());
    }

    // Move back or un promote piece
    if (move.isPromotion()) {
        board.addPieceToSquare<SideToMove>(PieceType::PAWN, move.fromSquare());
        board.removePieceFromSquare<SideToMove>(move.promotionPiece(), move.toSquare());
    } else {
        board.movePiece<SideToMove>(move.movingPiece(), move.toSquare(), move.fromSquare());
    }

    // If move was a castling move we need to move rook as well as king
    if (move.isKingsideCastle()) {
        if constexpr (SideToMove == Color::WHITE) {
            board.movePiece<Color::WHITE>(PieceType::ROOK, Square::F1, Square::H1);
        } else { // SideToMove == Color::BLACK
            board.movePiece<Color::BLACK>(PieceType::ROOK, Square::F8, Square::H8);
        }
    } else if (move.isQueensideCastle()) {
        if constexpr (SideToMove == Color::WHITE) {
            board.movePiece<Color::WHITE>(PieceType::ROOK, Square::D1, Square::A1);
        } else { // SideToMove == Color::BLACK
            board.movePiece<Color::BLACK>(PieceType::ROOK, Square::D8, Square::A8);
        }
    }

    // Finally, restore the previous board state values.
    board.setCastlingRights(undo.prev_castling_rights);
    board.setEnPassantSquare(undo.prev_en_passant_square);
    board.setHalfmoveClock(undo.prev_halfmove_clock);
    board.setFullmoveNumber(undo.prev_fullmove_number);
    board.calculateOccupancies();
    board.toggleSideToMove();
}

} // namespace internal

class MoveProcessor {
    std::array<internal::MoveUndo, internal::MAX_DEPTH> undo_history_{};
    size_t                                              undo_history_pointer_{0};

public:
    void applyMove(BoardState& board, const Move& move) {
        // Capture current state.
        internal::MoveUndo undo;
        undo.prev_castling_rights            = board.getCastlingRights();
        undo.prev_en_passant_square          = board.getEnPassantSquare();
        undo.prev_halfmove_clock             = board.getHalfmoveClock();
        undo.prev_fullmove_number            = board.getFullmoveNumber();
        undo.moving_side                     = board.getSideToMove();
        undo_history_[undo_history_pointer_] = undo;
        ++undo_history_pointer_;

        board.isWhiteMove() ? internal::applyMove<Color::WHITE>(board, move)
                            : internal::applyMove<Color::BLACK>(board, move);
    }

    void undoMove(BoardState& board, const Move& move) noexcept {
        internal::MoveUndo undo = undo_history_[undo_history_pointer_ - 1];
        --undo_history_pointer_;
        if (undo.moving_side == Color::WHITE) {
            internal::undoMove<Color::WHITE>(board, move, undo);
        } else {
            internal::undoMove<Color::BLACK>(board, move, undo);
        }
    }

    void resetHistory() { undo_history_pointer_ = 0; }
};

} // namespace bitcrusher

#endif // BITCRUSHER_MOVE_PROCESSOR_HPP
