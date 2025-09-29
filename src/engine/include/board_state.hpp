#ifndef BITCRUSHER_BOARD_STATE_HPP
#define BITCRUSHER_BOARD_STATE_HPP

#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include "bitboard_utils.hpp"
#include "zobrist_hash_keys.hpp"
#include <array>
#include <cassert>

namespace bitcrusher {

const std::array<Piece, PIECE_COUNT_PER_SIDE> WHITE_PIECES{
    Piece::WHITE_PAWN, Piece::WHITE_KNIGHT, Piece::WHITE_BISHOP,
    Piece::WHITE_ROOK, Piece::WHITE_QUEEN,  Piece::WHITE_KING,
};

const std::array<Piece, PIECE_COUNT_PER_SIDE> BLACK_PIECES{
    Piece::BLACK_PAWN, Piece::BLACK_KNIGHT, Piece::BLACK_BISHOP,
    Piece::BLACK_ROOK, Piece::BLACK_QUEEN,  Piece::BLACK_KING,
};

enum class OccupancyPolicy : bool { UPDATE, LEAVE };
enum class HashPolicy : bool { UPDATE, LEAVE };

class BoardState final {
public:
    // Bitboard accessors.
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
        if (square_bb & (bitboards_[Piece::BLACK_KING] | bitboards_[Piece::WHITE_KING])) {
            return PieceType::KING;
        }

        return PieceType::NONE;
    }

    void setZobristHash(uint64_t hash) { zobrist_hash_ = hash; }

    [[nodiscard]] uint64_t getZobristHash() const { return zobrist_hash_; }

    // Adding, removing and moving pieces.
    
    template <Color           Side,
              OccupancyPolicy UpdateOccupancy = OccupancyPolicy::UPDATE,
              HashPolicy      UpdateHash      = HashPolicy::UPDATE>
    constexpr void addPieceToSquare(PieceType piece_t, Square square) noexcept {
        Piece piece = convert::toPiece<Side>(piece_t);
        utils::setSquare(bitboards_[piece], square);
        if constexpr (UpdateOccupancy == OccupancyPolicy::UPDATE) {
            if constexpr (Side == Color::WHITE) {
                utils::setSquare(white_occupancy_, square);
            } else { // Side == Color::BLACK
                utils::setSquare(black_occupancy_, square);
            }
            utils::setSquare(all_occupancy_, square);
            utils::clearSquare(empty_squares_, square);
        }
        if constexpr (UpdateHash == HashPolicy::UPDATE) {
            zobrist_hash_ ^= ZobristKeys::getPieceSquareKey(piece, square);
        }
    }

    template <Color           Side,
              OccupancyPolicy UpdateOccupancy = OccupancyPolicy::UPDATE,
              HashPolicy      UpdateHash      = HashPolicy::UPDATE>
    constexpr void removePieceFromSquare(PieceType piece_t, Square square) noexcept {
        Piece piece = convert::toPiece<Side>(piece_t);
        utils::clearSquare(bitboards_[piece], square);
        if constexpr (UpdateOccupancy == OccupancyPolicy::UPDATE) {
            if constexpr (Side == Color::WHITE) {
                utils::clearSquare(white_occupancy_, square);
            } else { // Side == Color::BLACK
                utils::clearSquare(black_occupancy_, square);
            }
            utils::clearSquare(all_occupancy_, square);
            utils::setSquare(empty_squares_, square);
        }
        if constexpr (UpdateHash == HashPolicy::UPDATE) {
            zobrist_hash_ ^= ZobristKeys::getPieceSquareKey(piece, square);
        }
    }

    template <Color           Side,
              OccupancyPolicy UpdateOccupancy = OccupancyPolicy::UPDATE,
              HashPolicy      UpdateHash      = HashPolicy::UPDATE>
    constexpr void movePiece(PieceType piece_t, Square source, Square destination) noexcept {
        Piece    piece                = convert::toPiece<Side>(piece_t);
        uint64_t source_bitboard      = convert::toBitboard(source);
        uint64_t destination_bitboard = convert::toBitboard(destination);
        assert((bitboards_[piece] & source_bitboard) &&
               (bitboards_[piece] & destination_bitboard) == 0);
        uint64_t source_and_destination_bitboard = (source_bitboard | destination_bitboard);
        bitboards_[piece] ^= source_and_destination_bitboard;
        if constexpr (UpdateOccupancy == OccupancyPolicy::UPDATE) {
            if constexpr (Side == Color::WHITE) {
                white_occupancy_ ^= source_and_destination_bitboard;
            } else { // Side == Color::BLACK
                black_occupancy_ ^= source_and_destination_bitboard;
            }
            all_occupancy_ ^= source_and_destination_bitboard;
            empty_squares_ ^= source_and_destination_bitboard;
        }
        if constexpr (UpdateHash == HashPolicy::UPDATE) {
            zobrist_hash_ ^= ZobristKeys::getPieceSquareKey(piece, source);
            zobrist_hash_ ^= ZobristKeys::getPieceSquareKey(piece, destination);
        }
    }

    // Side to move
    [[nodiscard]] constexpr bool isWhiteMove() const noexcept {
        return side_to_move_ == Color::WHITE;
    }

    template <HashPolicy UpdateHash = HashPolicy::UPDATE>
    constexpr void setSideToMove(Color value) noexcept {
        if constexpr (UpdateHash == HashPolicy::UPDATE) {
            if (side_to_move_ != value) {
                zobrist_hash_ ^= ZobristKeys::getIsBlackMoveKey();
            }
        }
        side_to_move_ = value;
    }

    template <HashPolicy UpdateHash = HashPolicy::UPDATE> void toggleSideToMove() noexcept {
        side_to_move_ = ! side_to_move_;
        if constexpr (UpdateHash == HashPolicy::UPDATE) {
            zobrist_hash_ ^= ZobristKeys::getIsBlackMoveKey();
        }
    }

    // Castling rights
    template <CastlingRights Rights, HashPolicy UpdateHash = HashPolicy::UPDATE>
    constexpr void addCastlingRights() noexcept {
        assert(! hasCastlingRights<Rights>());
        castling_rights_ |= Rights;
        if constexpr (UpdateHash == HashPolicy::UPDATE) {
            if constexpr (SingularCastlingRight<Rights>) {
                zobrist_hash_ ^= ZobristKeys::getCastlingRightsKey<Rights>();
            } else {
                if constexpr (Rights == CastlingRights::WHITE_CASTLING_RIGHTS) {
                    zobrist_hash_ ^=
                        ZobristKeys::getCastlingRightsKey<CastlingRights::WHITE_KINGSIDE>();
                    zobrist_hash_ ^=
                        ZobristKeys::getCastlingRightsKey<CastlingRights::WHITE_QUEENSIDE>();
                } else if constexpr (Rights == CastlingRights::BLACK_CASTLING_RIGHTS) {
                    zobrist_hash_ ^=
                        ZobristKeys::getCastlingRightsKey<CastlingRights::BLACK_KINGSIDE>();
                    zobrist_hash_ ^=
                        ZobristKeys::getCastlingRightsKey<CastlingRights::BLACK_QUEENSIDE>();
                }
            }
        }
    }

    template <CastlingRights Rights, HashPolicy UpdateHash = HashPolicy::UPDATE>
    constexpr void removeCastlingRights() noexcept {
        CastlingRights prev_castling_rights = castling_rights_;
        castling_rights_ &= ~Rights;
        if constexpr (UpdateHash == HashPolicy::UPDATE) {
            if constexpr ((Rights & CastlingRights::WHITE_KINGSIDE) != CastlingRights::NONE) {
                if ((prev_castling_rights & CastlingRights::WHITE_KINGSIDE) !=
                    CastlingRights::NONE) {
                    zobrist_hash_ ^=
                        ZobristKeys::getCastlingRightsKey<CastlingRights::WHITE_KINGSIDE>();
                }
            }
            if constexpr ((Rights & CastlingRights::WHITE_QUEENSIDE) != CastlingRights::NONE) {
                if ((prev_castling_rights & CastlingRights::WHITE_QUEENSIDE) !=
                    CastlingRights::NONE) {
                    zobrist_hash_ ^=
                        ZobristKeys::getCastlingRightsKey<CastlingRights::WHITE_QUEENSIDE>();
                }
            }
            if constexpr ((Rights & CastlingRights::BLACK_KINGSIDE) != CastlingRights::NONE) {
                if ((prev_castling_rights & CastlingRights::BLACK_KINGSIDE) !=
                    CastlingRights::NONE) {
                    zobrist_hash_ ^=
                        ZobristKeys::getCastlingRightsKey<CastlingRights::BLACK_KINGSIDE>();
                }
            }
            if constexpr ((Rights & CastlingRights::BLACK_QUEENSIDE) != CastlingRights::NONE) {
                if ((prev_castling_rights & CastlingRights::BLACK_QUEENSIDE) !=
                    CastlingRights::NONE) {
                    zobrist_hash_ ^=
                        ZobristKeys::getCastlingRightsKey<CastlingRights::BLACK_QUEENSIDE>();
                }
            }
        }
    }

    [[nodiscard]] constexpr bool hasAnyCastlingRights() const {
        return castling_rights_ != CastlingRights::NONE;
    }

    template <CastlingRights Rights>
    [[nodiscard]] constexpr bool hasCastlingRights() const noexcept {
        if constexpr (SingularCastlingRight<Rights>) {
            // For singular rights: check if present
            return static_cast<bool>(castling_rights_ & Rights);
        } else {
            // For composite rights: check if ALL are present
            return (castling_rights_ & Rights) == Rights;
        }
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
    template <HashPolicy UpdateHash = HashPolicy::UPDATE>
    constexpr void setEnPassantSquare(Square square) noexcept {
        if constexpr (UpdateHash == HashPolicy::UPDATE) { // Remove old en passant square from hash.
            zobrist_hash_ ^= ZobristKeys::getEnPassantKey(en_passant_square_);
        }
        en_passant_square_ = square;
        if constexpr (UpdateHash == HashPolicy::UPDATE) { // Add new en passant square to hash.
            zobrist_hash_ ^= ZobristKeys::getEnPassantKey(square);
        }
    }

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
        castling_rights_   = CastlingRights::NONE;
        en_passant_square_ = Square::NULL_SQUARE;
        fullmove_number_   = 1;
        halfmove_clock_    = 0;
        side_to_move_      = Color::WHITE;
        calculateOccupancies();
    }

    template <Color Side> [[nodiscard]] uint64_t getOwnOccupancy() const {

        if constexpr (Side == Color::WHITE) {
            return white_occupancy_;
        } else { // Side==Color::BLACK
            return black_occupancy_;
        }
    }

    template <Color Side> [[nodiscard]] uint64_t getOpponentOccupancy() const {
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
    uint16_t       fullmove_number_{1};
    uint8_t        halfmove_clock_{0};
    Square         en_passant_square_{Square::NULL_SQUARE};
    CastlingRights castling_rights_ = CastlingRights::NONE;
    Color          side_to_move_{Color::WHITE};

    // Occupancies
    uint64_t white_occupancy_{0};
    uint64_t black_occupancy_{0};
    uint64_t all_occupancy_{0};
    uint64_t empty_squares_{FULL_BITBOARD};

    uint64_t zobrist_hash_{0};
    // ZobristHasher haserh;
};

} // namespace bitcrusher

#endif // BITCRUSHER_BOARD_STATE_HPP
