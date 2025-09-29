#ifndef BITCRUSHER_MOVE_PROCESSOR_HPP
#define BITCRUSHER_MOVE_PROCESSOR_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "move.hpp"
#include <array>
#include <cassert>
#include <cstdint>

namespace bitcrusher {

namespace internal {

const int MAX_DEPTH = 300;

struct MoveUndo {
    CastlingRights prev_castling_rights{};
    Square         prev_en_passant_square{};
    uint8_t        prev_halfmove_clock{};
    uint16_t       prev_fullmove_number{};
    Color          moving_side{};
    uint64_t       zobrist_hash{};

    bool operator==(const MoveUndo& rhs) const = default;
};

template <Color SideToMove>
constexpr void updateCastlingRightsOnKingOrRookMove(BoardState& board, const Move& move) {
    if constexpr (SideToMove == Color::WHITE) {
        if (move.movingPiece() == PieceType::KING) {
            board.removeCastlingRights<CastlingRights::WHITE_CASTLING_RIGHTS>();
        } else if (move.movingPiece() == PieceType::ROOK) {
            if (move.fromSquare() == Square::H1) {
                board.removeCastlingRights<CastlingRights::WHITE_KINGSIDE>();
            } else if (move.fromSquare() == Square::A1) {
                board.removeCastlingRights<CastlingRights::WHITE_QUEENSIDE>();
            }
        }
    } else if constexpr (SideToMove == Color::BLACK) {
        if (move.movingPiece() == PieceType::KING) {
            board.removeCastlingRights<CastlingRights::BLACK_CASTLING_RIGHTS>();
        } else if (move.movingPiece() == PieceType::ROOK) {
            if (move.fromSquare() == Square::H8) {
                board.removeCastlingRights<CastlingRights::BLACK_KINGSIDE>();
            } else if (move.fromSquare() == Square::A8) {
                board.removeCastlingRights<CastlingRights::BLACK_QUEENSIDE>();
            }
        }
    }
}

template <Color SideToMove>
constexpr void updateCastlingRightsOnRookCapture(BoardState& board, const Square to_square) {
    if constexpr (SideToMove == Color::WHITE) { // Check if White is capturing a Black rook.
        if (to_square == Square::H8) {
            board.removeCastlingRights<CastlingRights::BLACK_KINGSIDE>();
        } else if (to_square == Square::A8) {
            board.removeCastlingRights<CastlingRights::BLACK_QUEENSIDE>();
        }
    } else { // Check if Black is capturing a White rook.
        if (to_square == Square::H1) {
            board.removeCastlingRights<CastlingRights::WHITE_KINGSIDE>();
        } else if (to_square == Square::A1) {
            board.removeCastlingRights<CastlingRights::WHITE_QUEENSIDE>();
        }
    }
}

template <Color SideToMove>
static constexpr void applyMove(BoardState& board, const Move& move) noexcept {
    // For en passant or capture remove enemy piece and updates castling rights.
    if (move.isEnPassant()) {
        board.removePieceFromSquare<! SideToMove>(
            PieceType::PAWN,
            move.toSquare() + ((SideToMove == Color::WHITE) ? BOARD_DIMENSION : -BOARD_DIMENSION));
    } else if (move.isCapture()) {
        internal::updateCastlingRightsOnRookCapture<SideToMove>(board, move.toSquare());
        board.removePieceFromSquare<! SideToMove>(move.capturedPiece(),
                                                              move.toSquare());
    }

    // Move or promote piece.
    if (move.isPromotion()) {
        board.removePieceFromSquare<SideToMove>(PieceType::PAWN, move.fromSquare());
        board.addPieceToSquare<SideToMove>(move.promotionPiece(), move.toSquare());
    } else {
        board.movePiece<SideToMove>(move.movingPiece(), move.fromSquare(),
                                                move.toSquare());
    }

    // If move was a castling move we need to move rook as well as king.
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
    // Update castling rights if moved king or rook.
    internal::updateCastlingRightsOnKingOrRookMove<SideToMove>(board, move);

    // Update en passant square.
    if constexpr (SideToMove == Color::WHITE) {
        board.setEnPassantSquare(
            (move.isPawnDoublePush()) ? move.fromSquare() - BOARD_DIMENSION : Square::NULL_SQUARE);
    } else {
        board.setEnPassantSquare(
            (move.isPawnDoublePush()) ? move.fromSquare() + BOARD_DIMENSION : Square::NULL_SQUARE);
    }

    // Update move counters.
    if (move.isPromotion() || move.isCapture() || (move.movingPiece() == PieceType::PAWN)) {
        board.resetHalfmoveClock();
    } else {
        board.incrementHalfmoveClock();
    }

    if constexpr (SideToMove == Color::BLACK) {
        board.incrementFullmoveNumber();
    }
    board.toggleSideToMove();
}

template <Color SideToMove>
constexpr void
undoMove(BoardState& board, const Move& move, const internal::MoveUndo& undo) noexcept {

    // Move back or un promote piece.
    if (move.isPromotion()) {
        board.addPieceToSquare<SideToMove, OccupancyPolicy::UPDATE, HashPolicy::LEAVE>(PieceType::PAWN, move.fromSquare());
        board.removePieceFromSquare<SideToMove, OccupancyPolicy::UPDATE, HashPolicy::LEAVE>(
            move.promotionPiece(),
                                                             move.toSquare());
    } else {
        board.movePiece<SideToMove, OccupancyPolicy::UPDATE, HashPolicy::LEAVE>(
            move.movingPiece(), move.toSquare(),
                                                 move.fromSquare());
    }

    // If move was a castling move we need to move rook as well as king.
    if (move.isKingsideCastle()) {
        if constexpr (SideToMove == Color::WHITE) {
            board.movePiece<Color::WHITE, OccupancyPolicy::UPDATE, HashPolicy::LEAVE>(
                PieceType::ROOK, Square::F1, Square::H1);
        } else { // SideToMove == Color::BLACK
            board.movePiece<Color::BLACK, OccupancyPolicy::UPDATE, HashPolicy::LEAVE>(
                PieceType::ROOK, Square::F8, Square::H8);
        }
    } else if (move.isQueensideCastle()) {
        if constexpr (SideToMove == Color::WHITE) {
            board.movePiece<Color::WHITE, OccupancyPolicy::UPDATE, HashPolicy::LEAVE>(
                PieceType::ROOK, Square::D1, Square::A1);
        } else { // SideToMove == Color::BLACK
            board.movePiece<Color::BLACK, OccupancyPolicy::UPDATE, HashPolicy::LEAVE>(
                PieceType::ROOK, Square::D8, Square::A8);
        }
    }

    // Restore enemy piece if move is en passant or capture.
    if (move.isEnPassant()) {
        board.addPieceToSquare<! SideToMove, OccupancyPolicy::UPDATE, HashPolicy::LEAVE>(
            PieceType::PAWN,
            move.toSquare() + ((SideToMove == Color::WHITE) ? BOARD_DIMENSION : -BOARD_DIMENSION));
    } else if (move.isCapture()) {
        board.addPieceToSquare<! SideToMove, OccupancyPolicy::UPDATE, HashPolicy::LEAVE>(
            move.capturedPiece(), move.toSquare());
    }

    // Finally, restore the previous board state values.
    board.setCastlingRights(undo.prev_castling_rights);
    board.setEnPassantSquare<HashPolicy::LEAVE>(undo.prev_en_passant_square);
    board.setHalfmoveClock(undo.prev_halfmove_clock);
    board.setFullmoveNumber(undo.prev_fullmove_number);
    board.toggleSideToMove<HashPolicy::LEAVE>();
    board.setZobristHash(undo.zobrist_hash);
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
        undo.zobrist_hash                    = board.getZobristHash();
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
