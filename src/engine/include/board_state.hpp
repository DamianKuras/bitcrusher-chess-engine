#ifndef BITCRUSHER_BOARD_STATE_HPP
#define BITCRUSHER_BOARD_STATE_HPP

#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include "bitboard_utils.hpp"
#include <array>

namespace bitcrusher {

const std::array<Piece, PIECE_COUNT_PER_SIDE> WHITE_PIECES{
    Piece::WHITE_PAWN, Piece::WHITE_KNIGHT, Piece::WHITE_BISHOP,
    Piece::WHITE_ROOK, Piece::WHITE_QUEEN,  Piece::WHITE_KING,
};

const std::array<Piece, PIECE_COUNT_PER_SIDE> BLACK_PIECES{
    Piece::BLACK_PAWN, Piece::BLACK_KNIGHT, Piece::BLACK_BISHOP,
    Piece::BLACK_ROOK, Piece::BLACK_QUEEN,  Piece::BLACK_KING,
};

class BoardState final {
public:
    /* Bitboard accessors */
    [[nodiscard]] constexpr uint64_t getBitboard(Piece piece) const noexcept {
        return bitboards_[piece];
    }

    template <PieceType Piece, Color C>
    [[nodiscard]] constexpr uint64_t getBitboard() const noexcept {
        return bitboards_[convert::toPiece<C, Piece>()];
    }

    template <Color C> [[nodiscard]] constexpr uint64_t getDiagonalSliders() const noexcept {
        return getBitboard<PieceType::BISHOP, C>() | getBitboard<PieceType::QUEEN, C>();
    }

    template <Color C>
    [[nodiscard]] constexpr uint64_t getHorizontalVerticalSliders() const noexcept {
        return getBitboard<PieceType::ROOK, C>() | getBitboard<PieceType::QUEEN, C>();
    }

    [[nodiscard]] constexpr bool isPieceOnSquare(Piece piece, Square square) const noexcept {
        return utils::isSquareSet(bitboards_[piece], square);
    }

    [[nodiscard]] constexpr Piece getPieceOnSquare(Square sq) const noexcept {
        uint64_t square_bb = convert::toBitboard(sq);

        for (auto piece : WHITE_PIECES) {
            if (square_bb & getBitboard(piece)) {
                return piece;
            }
        }

        for (auto piece : BLACK_PIECES) {
            if (square_bb & getBitboard(piece)) {
                return piece;
            }
        }

        return Piece::NONE;
    }

    [[nodiscard]] constexpr PieceType getPieceTypeOnSquare(Square sq) const noexcept {
        uint64_t square_bb = convert::toBitboard(sq);
        if (square_bb & (bitboards_[Piece::BLACK_PAWN] | bitboards_[Piece::WHITE_PAWN])) {
            return PieceType::PAWN;
        }
        if (square_bb & (bitboards_[Piece::BLACK_KNIGHT] | bitboards_[Piece::WHITE_KNIGHT])) {
            return PieceType::KNIGHT;
        }
        if (square_bb & (bitboards_[Piece::BLACK_BISHOP] | bitboards_[Piece::WHITE_BISHOP])) {
            return PieceType::BISHOP;
        }
        if (square_bb & (bitboards_[Piece::BLACK_ROOK] | bitboards_[Piece::WHITE_ROOK])) {
            return PieceType::ROOK;
        }
        if (square_bb & (bitboards_[Piece::BLACK_QUEEN] | bitboards_[Piece::WHITE_QUEEN])) {
            return PieceType::QUEEN;
        }

        return PieceType::NONE;
    }

    // Adding, removing and moving pieces

    // sets piece on square without changing occupancy usefull when you want to
    // recalculate occupancy once after multiple changes otherwise use add

    template <Color Side>
    constexpr void removePieceFromSquare(PieceType piece_t, Square square) noexcept {
        Piece piece = convert::toPiece<Side>(piece_t);
        utils::clearSquare(bitboards_[piece], square);
    }

    template <Color Side>
    constexpr void addPieceToSquare(PieceType piece_t, Square square) noexcept {
        Piece piece = convert::toPiece<Side>(piece_t);
        utils::setSquare(bitboards_[piece], square);
        calculateOccupancies();
    }

    constexpr void addPieceToSquare(Piece piece, Square square) noexcept {
        utils::setSquare(bitboards_[piece], square);
        calculateOccupancies();
    }

    template <Color SideToMove>
    constexpr void movePiece(PieceType piece_t, Square source, Square destination) noexcept {
        Piece    piece = convert::toPiece<SideToMove>(piece_t);
        uint64_t source_and_destination_bitboard =
            (convert::toBitboard(source) | convert::toBitboard(destination));
        bitboards_[piece] ^= source_and_destination_bitboard;
    }

    // Side to move
    [[nodiscard]] constexpr bool isWhiteMove() const noexcept {
        return side_to_move_ == Color::WHITE;
    }

    constexpr void setSideToMove(Color value) noexcept { side_to_move_ = value; }

    void toggleSideToMove() noexcept { side_to_move_ = ! side_to_move_; }

    // Castling rights
    constexpr void removeWhiteCastlingRights() noexcept {
        castling_rights_ &= ~(CastlingRights::WHITE_KINGSIDE | CastlingRights::WHITE_QUEENSIDE);
    }

    constexpr void removeWhiteKingsideCastlingRight() noexcept {
        castling_rights_ &= ~(CastlingRights::WHITE_KINGSIDE);
    }

    constexpr void removeWhiteQueensideCastlingRight() noexcept {
        castling_rights_ &= ~(CastlingRights::WHITE_QUEENSIDE);
    }

    constexpr void addWhiteQueensideCastlingRight() noexcept {
        castling_rights_ |= CastlingRights::WHITE_QUEENSIDE;
    }

    constexpr void addWhiteKingsideCastlingRight() noexcept {
        castling_rights_ |= CastlingRights::WHITE_KINGSIDE;
    }

    [[nodiscard]] constexpr bool hasWhiteQueensideCastlingRight() const noexcept {
        return static_cast<bool>(castling_rights_ & CastlingRights::WHITE_QUEENSIDE);
    }

    [[nodiscard]] constexpr bool hasWhiteKingsideCastlingRight() const noexcept {
        return static_cast<bool>(castling_rights_ & CastlingRights::WHITE_KINGSIDE);
    }

    constexpr void removeBlackCastlingRights() noexcept {
        castling_rights_ &= ~(CastlingRights::BLACK_KINGSIDE | CastlingRights::BLACK_QUEENSIDE);
    }

    constexpr void removeBlackKingsideCastlingRight() noexcept {
        castling_rights_ &= ~CastlingRights::BLACK_KINGSIDE;
    }

    constexpr void removeBlackQueensideCastlingRight() noexcept {
        castling_rights_ &= ~CastlingRights::BLACK_QUEENSIDE;
    }

    constexpr void addBlackQueensideCastlingRight() noexcept {
        castling_rights_ |= CastlingRights::BLACK_QUEENSIDE;
    }

    constexpr void addBlackKingsideCastlingRight() noexcept {
        castling_rights_ |= CastlingRights::BLACK_KINGSIDE;
    }

    [[nodiscard]] constexpr bool hasBlackQueensideCastlingRight() const noexcept {
        return static_cast<bool>(castling_rights_ & CastlingRights::BLACK_QUEENSIDE);
    }

    [[nodiscard]] constexpr bool hasBlackKingsideCastlingRight() const noexcept {
        return static_cast<bool>(castling_rights_ & CastlingRights::BLACK_KINGSIDE);
    }

    // Counters
    constexpr void setHalfmoveClock(uint8_t halfmove_clock) noexcept {
        halfmove_clock_ = halfmove_clock;
    }

    void resetHalfmoveClock() noexcept { halfmove_clock_ = 0; }

    constexpr void incrementHalfmoveClock() noexcept { ++halfmove_clock_; }

    [[nodiscard]] constexpr uint8_t getHalfmoveClock() const noexcept { return halfmove_clock_; }

    constexpr void setFullmoveNumber(uint16_t fullmove_number) noexcept {
        fullmove_number_ = fullmove_number;
    }

    constexpr void incrementFullmoveNumber() noexcept { ++fullmove_number_; }

    [[nodiscard]] constexpr uint16_t getFullmoveNumber() const noexcept { return fullmove_number_; }

    // En passant management
    constexpr void setEnPassantSquare(Square square) noexcept { en_passant_square_ = square; }

    [[nodiscard]] constexpr Square getEnPassantSquare() const noexcept {
        return en_passant_square_;
    }

    // Occupancy
    [[nodiscard]] constexpr uint64_t generateWhiteOccupancy() const noexcept {
        return (bitboards_[Piece::WHITE_PAWN] | bitboards_[Piece::WHITE_KNIGHT] |
                bitboards_[Piece::WHITE_BISHOP] | bitboards_[Piece::WHITE_ROOK] |
                bitboards_[Piece::WHITE_QUEEN] | bitboards_[Piece::WHITE_KING]);
    }

    [[nodiscard]] constexpr uint64_t generateBlackOccupancy() const noexcept {
        return (bitboards_[Piece::BLACK_PAWN] | bitboards_[Piece::BLACK_KNIGHT] |
                bitboards_[Piece::BLACK_BISHOP] | bitboards_[Piece::BLACK_ROOK] |
                bitboards_[Piece::BLACK_QUEEN] | bitboards_[Piece::BLACK_KING]);
    }

    [[nodiscard]] Color getSideToMove() const noexcept { return side_to_move_; }

    constexpr void reset() noexcept {
        bitboards_.fill(0ULL);
        castling_rights_        = CastlingRights::NONE;
        en_passant_square_      = Square::NULL_SQUARE;
        fullmove_number_        = 1;
        halfmove_clock_         = 0;
        side_to_move_           = Color::WHITE;
        white_attacked_squares_ = EMPTY_BITBOARD;
        black_attacked_squares_ = EMPTY_BITBOARD;
        calculateOccupancies();
    }

    template <Color Side> [[nodiscard]] const uint64_t& getOwnOccupancy() const {
        if constexpr (Side == Color::WHITE) {
            return white_occupancy_;
        } else { // Side==Color::BLACK
            return black_occupancy_;
        }
    }

    template <Color Side> [[nodiscard]] const uint64_t& getOpponentOccupancy() const {
        if constexpr (Side == Color::WHITE) {
            return black_occupancy_;
        } else { // Side==Color::BLACK
            return white_occupancy_;
        }
    }

    [[nodiscard]] const uint64_t& getAllOccupancy() const { return all_occupancy_; }

    [[nodiscard]] const uint64_t& getEmptySquares() const { return empty_squares_; }

    [[nodiscard]] bool hasEnPassant() const { return en_passant_square_ != Square::NULL_SQUARE; }

    [[nodiscard]] bool isEmpty(uint64_t squares_bitboard) const {
        return (squares_bitboard & getAllOccupancy()) == EMPTY_BITBOARD;
    }

    template <Color Side>
    [[nodiscard]] constexpr bool isNotAttackedByOpponent(uint64_t squares_bitboard,
                                                         uint64_t enemy_attacked_squares) const {
        return (squares_bitboard & enemy_attacked_squares) == EMPTY_BITBOARD;
    }

    template <Color Side> void updateOpponentAttackedSquares(uint64_t attacks) {
        if constexpr (Side == Color::WHITE) {
            black_attacked_squares_ = attacks;
        } else {
            white_attacked_squares_ = attacks;
        }
    }

    void setCastlingRights(CastlingRights rights) { castling_rights_ = rights; }

    [[nodiscard]] CastlingRights getCastlingRights() const { return castling_rights_; }

    bool operator==(const BoardState& rhs) const = default;

    constexpr void calculateOccupancies() {
        white_occupancy_ = generateWhiteOccupancy();
        black_occupancy_ = generateBlackOccupancy();
        all_occupancy_   = white_occupancy_ | black_occupancy_;
        empty_squares_   = ~(all_occupancy_);
    }

private:
    EnumIndexedArray<std::uint64_t, Piece, static_cast<std::size_t>(Piece::COUNT)> bitboards_;
    uint64_t       white_attacked_squares_{};
    uint64_t       black_attacked_squares_{};
    uint16_t       fullmove_number_{1};
    uint8_t        halfmove_clock_{0};
    Square         en_passant_square_{Square::NULL_SQUARE};
    CastlingRights castling_rights_ = CastlingRights::NONE;
    Color          side_to_move_{Color::WHITE};
    uint64_t       white_occupancy_{};
    uint64_t       black_occupancy_{};
    uint64_t       all_occupancy_{};
    uint64_t       empty_squares_{};
};

} // namespace bitcrusher

#endif // BITCRUSHER_BOARD_STATE_HPP
