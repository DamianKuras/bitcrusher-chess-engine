#ifndef BITCRUSHER_SHARED_MOVE_GENERATION_HPP
#define BITCRUSHER_SHARED_MOVE_GENERATION_HPP

#include "bitboard_enums.hpp"
#include "concepts.hpp"
#include "pext_bitboards.hpp"
#include <cstdint>

namespace bitcrusher {

/// @brief Move generation policy representing which types of moves to produce.
enum class MoveGenerationPolicy : bool {
    FULL,
    CAPTURES_ONLY,
};

/// @brief Creates moves given a target squares bitboard and an offset to calculate the source
/// square of the moving piece .
/// @tparam MoveT The type of move being created.
/// @tparam MovedOrPromotedToPiece The piece type moving or promoted to piece type for promotion
/// moves.
/// @tparam SideToMove The color of the side generating moves.
/// @tparam CapturedPiece The piece type being captured, if any (defaults to NONE for non-captures).
/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @param sink The move sink object that will store the created moves.
/// @param move_to_target_squares Bitboard with bits set for each destination square.
/// @param offset_to_create_target_square Offset subtracted from target squares to compute source
/// squares.
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

/// @brief Creates moves from a target squares bitboard with an explicit source square.
/// @tparam MoveT The type of move being created.
/// @tparam MovedOrPromotedToPiece The piece type moving or promoted to piece type for promotion
/// moves.
/// @tparam SideToMove The color of the side generating moves.
/// @tparam CapturedPiece The piece type being captured, if any (defaults to NONE for non-captures).
/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @param sink The move sink object that will store the created moves.
/// @param move_to_target_squares Bitboard with bits set for each destination square.
/// @param move_from The source square containing the piece for all generated moves.
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

            sink.template emplace<MoveT, PieceType::QUEEN, SideToMove, CapturedPiece>(move_from,
                                                                                      to_square);
            sink.template emplace<MoveT, PieceType::ROOK, SideToMove, CapturedPiece>(move_from,
                                                                                     to_square);
            sink.template emplace<MoveT, PieceType::BISHOP, SideToMove, CapturedPiece>(move_from,
                                                                                       to_square);
            sink.template emplace<MoveT, PieceType::KNIGHT, SideToMove, CapturedPiece>(move_from,
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

/// @brief Generates capture moves ordered by MVV-LVA (Most Valuable Victim - Least Valuable
/// Aggressor).
/// @tparam Side The Color of the side to move(Color::WHITE or Color::BLACK).
/// @tparam MovedPiece The piece type performing the capture/captures.
/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @param attacks_bitboard Bitboard containing all squares where legal captures can be made.
/// @param sink The move sink object that will store the generated capture moves.
/// @param board The current board state of the position.
/// @param offset_to_create_target_square Offset value subtracted from target squares to encode the
/// source square in the move representation.
template <Color Side, PieceType MovedPiece, MoveSink MoveSinkT>
void generateOrderedCapturesMVV_LVA(const uint64_t    attacks_bitboard,
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

/// @brief Generates capture moves ordered by MVV-LVA (Most Valuable Victim - Least Valuable
/// Aggressor). Produces captures for a single attacking piece.
/// @tparam Side The Color of the side to move(Color::WHITE or Color::BLACK).
/// @tparam MovedPiece The piece type performing the capture/captures.
/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @param attacks_bitboard Bitboard containing all squares where legal captures can be made.
/// @param sink The move sink object that will store the generated capture moves.
/// @param board The current board state of the position.
/// @param sq The square where the attacking piece is located.
template <Color Side, PieceType MovedPiece, MoveSink MoveSinkT>
void generateOrderedCapturesMVV_LVA(const uint64_t    attacks_bitboard,
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

/// @brief Generates promotion capture moves ordered by MVV-LVA (Most Valuable Victim - Least
/// Valuable Aggressor).
/// @tparam Side The Color of the side to move(Color::WHITE or Color::BLACK).
/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @param attacks_bitboard Bitboard containing all squares where legal captures can be made.
/// @param sink The move sink object that will store the generated capture moves.
/// @param board The current board state of the position.
/// @param offset_to_create_target_square Offset value subtracted from target squares to encode the
/// source square in the move representation.
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

#if defined(HAS_BMI2)
/// @brief Generates horizontal-vertical slider (rook pattern) attack bitboard using PEXT magic
/// bitboard technique. Produces captures for a single attacking piece.
/// @param square The Square where the horizontal-vertical slider is located.
/// @param occupancy Bitboard representing all occupied squares on the board.
/// @return Bitboard with all squares the horizontal-vertical slider can attack.
inline uint64_t getHorizontalVerticalAttacks(Square square, uint64_t occupancy) {
    occupancy &= PextBitboards::rook_masks[static_cast<int>(square)];
    int index = _pext_u64(occupancy, PextBitboards::rook_masks[static_cast<int>(square)]);
    return PextBitboards::attack_table[PextBitboards::rook_index[static_cast<int>(square)] + index];
}

/// @brief Generate diagonal slider (bishop pattern) attack bitboard using PEXT magic bitboard
/// technique. Produces captures for a single attacking piece.
/// @param square The Square where the horizontal vertical slider is located.
/// @param occupancy Bitboard representing all occupied squares on the board.
/// @return Bitboard with all squares the diagonal slider can attack.
inline uint64_t getDiagonalAttacks(Square square, uint64_t occupancy) {
    occupancy &= PextBitboards::bishop_masks[static_cast<int>(square)];
    int index = _pext_u64(occupancy, PextBitboards::bishop_masks[static_cast<int>(square)]);
    return PextBitboards::attack_table[PextBitboards::bishop_index[static_cast<int>(square)] +
                                       index];
}
#endif

/// @brief Generates Diagonal Sliding Piece Moves ordered by MVV-LVA (Most Valuable Victim - Least
/// Valuable Aggressor).
/// @tparam MovedPieceT Type of the moved slider piece (Bisop or Queen).
/// @tparam Side Color of the sliding piece.
/// @tparam MoveGenerationP Move generation policy MoveGenerationPolicy.
/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @param source_squares Bitboard(uint64_t) with squares set corresponding to the diagonal sliders
/// that moves are generated for.
/// @param board BoardState of the position to generate moves for.
/// @param sink Object that will store/use generated moves.
/// @param restriction_mask Restriction masks restricting moves for all diagonal pieces in the
/// source_squares. Importantly pinned pieces can only move along they pin mask.
template <PieceType            MovedPieceT,
          Color                Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateDiagonalSlidingPieceMoves(uint64_t          source_squares,
                                       const BoardState& board,
                                       MoveSinkT&        sink,
                                       uint64_t          restriction_mask) {
#if defined(HAS_BMI2)
    while (source_squares != EMPTY_BITBOARD) {
        Square   piece_sq = utils::popFirstSetSquare(source_squares);
        uint64_t piece_bb = convert::toBitboard(piece_sq);

        uint64_t piece_attacks{};
        piece_attacks = getDiagonalAttacks(piece_sq, board.getAllOccupancy()) & restriction_mask;

        generateOrderedCapturesMVV_LVA<Side, MovedPieceT>(piece_attacks, sink, board, piece_sq);
        if constexpr (MoveGenerationP == MoveGenerationPolicy::FULL) {
            uint64_t quiet_moves = piece_attacks & board.getEmptySquares();
            createMovesFromBitboard<MoveType::QUIET, MovedPieceT, Side>(sink, quiet_moves,
                                                                        piece_sq);
        }
    }
#else
    while (source_squares != EMPTY_BITBOARD) {
        Square   piece_sq = utils::popFirstSetSquare(source_squares);
        uint64_t piece_bb = convert::toBitboard(piece_sq);
        uint64_t piece_attacks =
            generateDiagonalAttacks(piece_bb, board.getAllOccupancy()) & restriction_mask;
        generateOrderedCapturesMVV_LVA<Side, MovedPieceT>(piece_attacks, sink, board, piece_sq);
        if constexpr (MoveGenerationP == MoveGenerationPolicy::FULL) {
            uint64_t quiet_moves = piece_attacks & board.getEmptySquares();
            createMovesFromBitboard<MoveType::QUIET, MovedPieceT, Side>(sink, quiet_moves,
                                                                        piece_sq);
        }
    }
#endif
}

/// @brief Generates Horizontal-Vertical Sliding Piece Moves ordered by MVV-LVA (Most Valuable
/// Victim - Least Valuable Aggressor).
/// @tparam MovedPieceT Type of the moved slider piece (Rook or Queen).
/// @tparam Side
/// @tparam MoveGenerationP
/// @tparam MoveSinkT
/// @param source_squares
/// @param board
/// @param sink
/// @param restriction_mask
template <PieceType            MovedPieceT,
          Color                Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateHorizontalVerticalSlidingPieceMoves(uint64_t          source_squares,
                                                 const BoardState& board,
                                                 MoveSinkT&        sink,
                                                 uint64_t          restriction_mask) {

#if defined(HAS_BMI2)
    while (source_squares != EMPTY_BITBOARD) {
        Square   piece_sq = utils::popFirstSetSquare(source_squares);
        uint64_t piece_bb = convert::toBitboard(piece_sq);

        uint64_t piece_attacks{};
        piece_attacks =
            getHorizontalVerticalAttacks(piece_sq, board.getAllOccupancy()) & restriction_mask;

        generateOrderedCapturesMVV_LVA<Side, MovedPieceT>(piece_attacks, sink, board, piece_sq);
        if constexpr (MoveGenerationP == MoveGenerationPolicy::FULL) {
            uint64_t quiet_moves = piece_attacks & board.getEmptySquares();
            createMovesFromBitboard<MoveType::QUIET, MovedPieceT, Side>(sink, quiet_moves,
                                                                        piece_sq);
        }
    }
#else
    while (source_squares != EMPTY_BITBOARD) {
        Square   piece_sq = utils::popFirstSetSquare(source_squares);
        uint64_t piece_bb = convert::toBitboard(piece_sq);
        uint64_t piece_attacks =
            generateHorizontalVerticalAttacks(piece_bb, board.getAllOccupancy()) & restriction_mask;
        generateOrderedCapturesMVV_LVA<Side, MovedPieceT>(piece_attacks, sink, board, piece_sq);

        if constexpr (MoveGenerationP == MoveGenerationPolicy::FULL) {
            uint64_t quiet_moves = piece_attacks & board.getEmptySquares();
            createMovesFromBitboard<MoveType::QUIET, MovedPieceT, Side>(sink, quiet_moves,
                                                                        piece_sq);
        }
    }
#endif
}

} // namespace bitcrusher
#endif // BITCRUSHER_SHARED_MOVE_GENERATION_HPP