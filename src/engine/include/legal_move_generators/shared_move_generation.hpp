#ifndef BITCRUSHER_SHARED_MOVE_GENERATION_HPP
#define BITCRUSHER_SHARED_MOVE_GENERATION_HPP

#include "bitboard_enums.hpp"
#include "concepts.hpp"
#include "pext_bitboards.hpp"
#include <cstdint>

namespace bitcrusher {

enum class MoveGenerationPolicy : bool {
    FULL,
    CAPTURES_ONLY,
};

template <Color Side, PieceType MovedPiece, MoveSink MoveSinkT>
void generateOrderedCaptures(const uint64_t    attacks_bitboard,
                             MoveSinkT&        sink,
                             const BoardState& board,
                             int               offset_to_create_target_square) {

    auto process_piece = [&]<PieceType CapturedPiece>() {
        uint64_t piece_type_captures =
            attacks_bitboard & board.getBitboard<CapturedPiece, ! Side>();
        createMovesFromBitboard<MoveType::CAPTURE, MovedPiece, Side, CapturedPiece>(
            sink, piece_type_captures, offset_to_create_target_square);
    };
    process_piece.template operator()<PieceType::QUEEN>();
    process_piece.template operator()<PieceType::ROOK>();
    process_piece.template operator()<PieceType::BISHOP>();
    process_piece.template operator()<PieceType::KNIGHT>();
    process_piece.template operator()<PieceType::PAWN>();
}

template <Color Side, PieceType MovedPiece, MoveSink MoveSinkT>
void generateOrderedCaptures(const uint64_t    attacks_bitboard,
                             MoveSinkT&        sink,
                             const BoardState& board,
                             Square            sq) {

    auto process_piece = [&]<PieceType CapturedPiece>() {
        uint64_t piece_type_captures =
            attacks_bitboard & board.getBitboard<CapturedPiece, ! Side>();
        createMovesFromBitboard<MoveType::CAPTURE, MovedPiece, Side, CapturedPiece>(
            sink, piece_type_captures, sq);
    };
    process_piece.template operator()<PieceType::QUEEN>();
    process_piece.template operator()<PieceType::ROOK>();
    process_piece.template operator()<PieceType::BISHOP>();
    process_piece.template operator()<PieceType::KNIGHT>();
    process_piece.template operator()<PieceType::PAWN>();
}

template <Color Side, MoveSink MoveSinkT>
void generateOrderedPromotionCaptures(const uint64_t    attacks_bitboard,
                                      MoveSinkT&        sink,
                                      const BoardState& board,
                                      int               offset_to_create_target_square) {

    auto process_piece = [&]<PieceType CapturedPiece>() {
        uint64_t piece_type_captures =
            attacks_bitboard & board.getBitboard<CapturedPiece, ! Side>();
        createMovesFromBitboard<MoveType::PROMOTION_CAPTURE, PieceType::PAWN, Side, CapturedPiece>(
            sink, piece_type_captures, offset_to_create_target_square);
    };

    process_piece.template operator()<PieceType::QUEEN>();
    process_piece.template operator()<PieceType::ROOK>();
    process_piece.template operator()<PieceType::BISHOP>();
    process_piece.template operator()<PieceType::KNIGHT>();
    process_piece.template operator()<PieceType::PAWN>();
}

inline uint64_t getRookAttacks(Square square, uint64_t occupancy) {
    occupancy &= PextBitboards::rook_masks[static_cast<int>(square)];
    int index = _pext_u64(occupancy, PextBitboards::rook_masks[static_cast<int>(square)]);
    return PextBitboards::attack_table[PextBitboards::rook_index[static_cast<int>(square)] + index];
}

inline uint64_t getBishopAttacks(Square square, uint64_t occupancy) {
    occupancy &= PextBitboards::bishop_masks[static_cast<int>(square)];
    int index = _pext_u64(occupancy, PextBitboards::bishop_masks[static_cast<int>(square)]);
    return PextBitboards::attack_table[PextBitboards::bishop_index[static_cast<int>(square)] +
                                       index];
}

template <PieceType            MovedPieceT,
          Color                Side,
          auto                 DirectionalAttackGen,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
    requires DirectionalAttackGenerator<DirectionalAttackGen> && MoveSink<MoveSinkT>
void generateSlidingPieceMoves(uint64_t          source_squares,
                               const BoardState& board,
                               MoveSinkT&        sink,
                               uint64_t          restriction_mask) {
    while (source_squares != EMPTY_BITBOARD) {
        Square   piece_sq = utils::popFirstSetSquare(source_squares);
        uint64_t piece_bb = convert::toBitboard(piece_sq);
 
        uint64_t piece_attacks{};
        if constexpr (DirectionalAttackGen == generateHorizontalVerticalAttacks) {
            piece_attacks = getRookAttacks(piece_sq, board.getAllOccupancy()) & restriction_mask;
        } else {
            piece_attacks = getBishopAttacks(piece_sq, board.getAllOccupancy()) & restriction_mask;
        }

        generateOrderedCaptures<Side, MovedPieceT>(piece_attacks, sink, board, piece_sq);
        if constexpr (MoveGenerationP == MoveGenerationPolicy::FULL) {
            uint64_t quiet_moves = piece_attacks & board.getEmptySquares();
            createMovesFromBitboard<MoveType::QUIET, MovedPieceT, Side>(sink, quiet_moves,
                                                                        piece_sq);
        }
    }
}

// Creates moves given a bitboard and an offset to calculate the source square.
template <MoveType  MoveT,
          PieceType MovedOrPromotedToPiece,
          Color     SideToMove,
          PieceType CapturedPiece = PieceType::NONE,
          MoveSink  MoveSinkT>
inline void createMovesFromBitboard(MoveSinkT& sink,
                                    uint64_t   move_to_target_squares,
                                    int        offset_to_create_target_square) {
    if constexpr (MoveT == MoveType::PROMOTION || MoveT == MoveType::PROMOTION_CAPTURE) {
        while (move_to_target_squares != EMPTY_BITBOARD) {
            Square to_square = utils::popFirstSetSquare(move_to_target_squares);

            sink.template emplace<MoveT, PieceType::QUEEN, SideToMove, CapturedPiece>(
                to_square - offset_to_create_target_square, to_square);
            sink.template emplace<MoveT, PieceType::ROOK, SideToMove, CapturedPiece>(
                to_square - offset_to_create_target_square, to_square);
            sink.template emplace<MoveT, PieceType::BISHOP, SideToMove, CapturedPiece>(
                to_square - offset_to_create_target_square, to_square);
            sink.template emplace<MoveT, PieceType::KNIGHT, SideToMove, CapturedPiece>(
                to_square - offset_to_create_target_square, to_square);
        }
    } else {
        while (move_to_target_squares != EMPTY_BITBOARD) {
            Square to_square = utils::popFirstSetSquare(move_to_target_squares);
            sink.template emplace<MoveT, MovedOrPromotedToPiece, SideToMove, CapturedPiece>(
                to_square - offset_to_create_target_square, to_square);
        }
    }
}

// Creates moves given a bitboard and an explicit source square.
template <MoveType  MoveT,
          PieceType MovedOrPromotedToPiece,
          Color     SideToMove,
          PieceType CapturedPiece = PieceType::NONE,
          MoveSink  MoveSinkT>
inline void
createMovesFromBitboard(MoveSinkT& sink, uint64_t move_to_target_squares, Square move_from) {

    if constexpr (MoveT == MoveType::PROMOTION || MoveT == MoveType::PROMOTION_CAPTURE) {
        while (move_to_target_squares != EMPTY_BITBOARD) {
            Square to_square = utils::popFirstSetSquare(move_to_target_squares);

            sink.template emplace<MoveT, PieceType::QUEEN, CapturedPiece, SideToMove>(move_from,
                                                                                      to_square);
            sink.template emplace<MoveT, PieceType::ROOK, CapturedPiece, SideToMove>(move_from,
                                                                                     to_square);
            sink.template emplace<MoveT, PieceType::BISHOP, CapturedPiece, SideToMove>(move_from,
                                                                                       to_square);
            sink.template emplace<MoveT, PieceType::KNIGHT, CapturedPiece, SideToMove>(move_from,
                                                                                       to_square);
        }

    } else {
        while (move_to_target_squares != EMPTY_BITBOARD) {
            Square to_square = utils::popFirstSetSquare(move_to_target_squares);
            sink.template emplace<MoveT, MovedOrPromotedToPiece, SideToMove, CapturedPiece>(
                move_from, to_square);
        }
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_SHARED_MOVE_GENERATION_HPP