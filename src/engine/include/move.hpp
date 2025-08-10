#ifndef BITCRUSHER_MOVE_HPP
#define BITCRUSHER_MOVE_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include <format>
#include <string>

namespace bitcrusher {

enum class MoveType : uint8_t {
    QUIET,
    DOUBLE_PAWN_PUSH,
    KINGSIDE_CASTLE,
    QUEENSIDE_CASTLE,
    CAPTURE,
    EN_PASSANT,
    PROMOTION,
    PROMOTION_CAPTURE,
    NULL_MOVE,
};

class Move final {
public:
    [[nodiscard]] constexpr Square fromSquare() const noexcept { return from_square_; }

    [[nodiscard]] constexpr Square toSquare() const noexcept { return to_square_; }

    [[nodiscard]] constexpr PieceType movingPiece() const noexcept { return moving_piece_; }

    [[nodiscard]] constexpr PieceType capturedPiece() const noexcept { return captured_piece_; }

    // In a promotion move, moving_piece_ represents the piece the pawn is
    // promoted to.
    [[nodiscard]] constexpr PieceType promotionPiece() const noexcept { return moving_piece_; }

    [[nodiscard]] constexpr bool isQuiet() const noexcept { return flag_ == MoveType::QUIET; }

    [[nodiscard]] constexpr bool isEnPassant() const noexcept {
        return flag_ == MoveType::EN_PASSANT;
    }

    [[nodiscard]] constexpr bool isCapture() const noexcept {
        return flag_ == MoveType::CAPTURE || flag_ == MoveType::PROMOTION_CAPTURE ||
               flag_ == MoveType::EN_PASSANT;
    }

    [[nodiscard]] constexpr bool isPromotion() const noexcept {
        return flag_ == MoveType::PROMOTION || flag_ == MoveType::PROMOTION_CAPTURE;
    }

    [[nodiscard]] constexpr bool isPawnDoublePush() const noexcept {
        return flag_ == MoveType::DOUBLE_PAWN_PUSH;
    }

    [[nodiscard]] constexpr bool isKingsideCastle() const noexcept {
        return flag_ == MoveType::KINGSIDE_CASTLE;
    }

    [[nodiscard]] bool isQueensideCastle() const noexcept {
        return flag_ == MoveType::QUEENSIDE_CASTLE;
    }

    [[nodiscard]] bool isNullMove() const noexcept { return flag_ == MoveType::NULL_MOVE; }

    // Factory functions for creating moves:
    [[nodiscard]] static Move
    createQuietMove(Square from, Square to, PieceType moved_piece) noexcept {
        return {from, to, moved_piece, MoveType::QUIET};
    }

    [[nodiscard]] static Move createDoublePawnPushMove(Square from, Square to) noexcept {
        return {from, to, PieceType::PAWN, MoveType::DOUBLE_PAWN_PUSH};
    };

    template <Color SideToMove, Side CastleSide>
    [[nodiscard]] static constexpr Move createCastlingMove() noexcept {
        if constexpr (SideToMove == Color::WHITE && CastleSide == Side::KINGSIDE) {
            return {Square::E1, Square::G1, PieceType::KING, MoveType::KINGSIDE_CASTLE};
        } else if constexpr (SideToMove == Color::WHITE && CastleSide == Side::QUEENSIDE) {
            return {Square::E1, Square::C1, PieceType::KING, MoveType::QUEENSIDE_CASTLE};
        } else if constexpr (SideToMove == Color::BLACK && CastleSide == Side::KINGSIDE) {
            return {Square::E8, Square::G8, PieceType::KING, MoveType::KINGSIDE_CASTLE};
        } else if constexpr (SideToMove == Color::BLACK && CastleSide == Side::QUEENSIDE) {
            return {Square::E8, Square::C8, PieceType::KING, MoveType::QUEENSIDE_CASTLE};
        }
    }

    [[nodiscard]] static Move createCaptureMove(Square    from,
                                                Square    to,
                                                PieceType moved_piece,
                                                PieceType captured_piece) noexcept {
        return {from, to, moved_piece, MoveType::CAPTURE, captured_piece};
    };

    [[nodiscard]] static Move createEnPassantMove(Square from, Square to) noexcept {
        return {from, to, PieceType::PAWN, MoveType::EN_PASSANT, PieceType::PAWN};
    };

    [[nodiscard]] static Move
    createPromotionMove(Square from, Square to, PieceType pawn_promoted_to) noexcept {
        return {from, to, pawn_promoted_to, MoveType::PROMOTION};
    }

    [[nodiscard]] static Move createPromotionCaptureMove(Square    from,
                                                         Square    to,
                                                         PieceType pawn_promoted_to,
                                                         PieceType captured) noexcept {
        return {from, to, pawn_promoted_to, MoveType::PROMOTION_CAPTURE, captured};
    }

    [[nodiscard]] bool isPromotionCapture() const noexcept {
        return flag_ == MoveType::PROMOTION_CAPTURE;
    }

    template <MoveType MoveT, PieceType MovedOrPromotedToPiece>
    static constexpr Move createMove(Square from, Square to, const BoardState& board) {

        if constexpr (MoveT == MoveType::CAPTURE) {
            return createCaptureMove(from, to, MovedOrPromotedToPiece,
                                     board.getPieceTypeOnSquare(to));
        }

        if constexpr (MoveT == MoveType::DOUBLE_PAWN_PUSH) {
            return createDoublePawnPushMove(from, to);
        }
        if constexpr (MoveT == MoveType::EN_PASSANT) {
            return createEnPassantMove(from, to);
        }
        if constexpr (MoveT == MoveType::KINGSIDE_CASTLE) {
            if (board.isWhiteMove()) {
                return createCastlingMove<Color::WHITE, Side::KINGSIDE>();
            }
            return createCastlingMove<Color::BLACK, Side::KINGSIDE>();
        }
        if constexpr (MoveT == MoveType::QUEENSIDE_CASTLE) {
            if (board.isWhiteMove()) {
                return createCastlingMove<Color::WHITE, Side::QUEENSIDE>();
            }
            return createCastlingMove<Color::BLACK, Side::QUEENSIDE>();
        }
        if constexpr (MoveT == MoveType::PROMOTION) {
            return createPromotionMove(from, to, MovedOrPromotedToPiece);
        }

        if constexpr (MoveT == MoveType::PROMOTION_CAPTURE) {
            return createPromotionCaptureMove(from, to, MovedOrPromotedToPiece,
                                              board.getPieceTypeOnSquare(to));
        }

        if constexpr (MoveT == MoveType::QUIET) {
            return createQuietMove(from, to, MovedOrPromotedToPiece);
        }
    }

    constexpr Move() noexcept                    = default;
    constexpr bool operator==(const Move&) const = default;

    static Move none() {
        return {Square::A1, Square::A1, PieceType::NONE, MoveType::NULL_MOVE, PieceType::NONE};
    }

private:
    Square    from_square_{};
    Square    to_square_{};
    PieceType moving_piece_{};
    MoveType  flag_{};
    PieceType captured_piece_{};

    Move(Square    from,
         Square    to,
         PieceType moved_piece,
         MoveType  flag,
         PieceType captured_piece = PieceType{}) noexcept
        : from_square_{from}, to_square_{to}, moving_piece_{moved_piece}, flag_{flag},
          captured_piece_{captured_piece} {}
};

[[nodiscard]] constexpr std::string toUci(const Move& move) {
    const auto [from_file, from_rank] = convert::toChars(move.fromSquare());
    const auto [to_file, to_rank]     = convert::toChars(move.toSquare());

    if (! move.isPromotion()) {
        return std::format("{}{}{}{}", from_file, from_rank, to_file, to_rank);
    }

    const char promo_char = convert::toPromotionUci(move.movingPiece());
    return std::format("{}{}{}{}{}", from_file, from_rank, to_file, to_rank, promo_char);
}

} // namespace bitcrusher

static_assert(std::is_trivially_copyable_v<bitcrusher::Move>, "Move must be trivially copyable");
static_assert(std::is_copy_constructible_v<bitcrusher::Move>, "Move must be copy constructible");

#endif // BITCRUSHER_MOVE_HPP
