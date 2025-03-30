#ifndef BITCRUSHER_CHECKS_PINS_DETECTION_HPP
#define BITCRUSHER_CHECKS_PINS_DETECTION_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "diagonal_bitboards.hpp"
#include "diagonal_slider_moves.hpp"
#include "horizontal_vertical_slider_moves.hpp"
#include "king_attacks.hpp"
#include "knight_attacks.hpp"
#include "pawn_attacks.hpp"
#include "restriction_context.hpp"

namespace bitcrusher {

template <Color Side> inline uint64_t generateSquaresAttacked(const BoardState& board) {
    uint64_t attacked_squares = 0ULL;

    // Pawns attacks
    attacked_squares |= generatePawnsAttacks<Side>(board.getBitboard<PieceType::PAWN, Side>());

    // Knights attacks
    attacked_squares |= generateKnightsAttacks(board.getBitboard<PieceType::KNIGHT, Side>());
    // Diagonal ray Pieces
    attacked_squares |=
        generateDiagonalAttacks(board.getDiagonalSliders<Side>(), board.getAllOccupancy());

    // Horizontal Vertical ray Pieces
    attacked_squares |= generateHorizontalVerticalAttacks(
        board.getHorizontalVerticalSliders<Side>(), board.getAllOccupancy());

    // King attacks
    attacked_squares |= generateKingAttacks(board.getBitboard<PieceType::KING, Side>());

    return attacked_squares;
}

template <typename AttackGenerator, SlidingPieceType SlidingPieceT>
void processSlidingPieceChecks(uint64_t            king_bitboard,
                               uint64_t            enemy_sliders,
                               uint64_t            line_occupancy,
                               uint64_t            line_mask,
                               uint64_t            our_occupancy,
                               AttackGenerator     generate_attacks,
                               RestrictionContext& restriction_context) {
    const uint64_t line_attacks    = generate_attacks(king_bitboard, line_occupancy, line_mask);
    const uint64_t slider_checkers = line_attacks & enemy_sliders;
    const uint64_t potential_pins  = line_attacks & our_occupancy;

    if (slider_checkers != EMPTY_BITBOARD) {
        restriction_context.checkers |= slider_checkers;
        restriction_context.check_block |= (line_attacks ^ slider_checkers);
    } else if (potential_pins != EMPTY_BITBOARD) { 
        const uint64_t beyond_ray = generate_attacks(potential_pins, line_occupancy, line_mask);
        if (beyond_ray & enemy_sliders) {
            if constexpr (SlidingPieceT == SlidingPieceType::DIAGONAL) {
                restriction_context.pinmask_diagonal |= potential_pins;
            } else { // SlidingPieceT == SlidingPieceType::HORIZONTAL_VERTICAL
                restriction_context.pinmask_horizontal_vertical |= potential_pins;
            }
        }
    }
}

template <Color SideToMove>
inline void updateRestrictionContext(const BoardState&   board,
                                     RestrictionContext& restriction_context) {
    restriction_context.reset();
    // constexpr Color opponentColor = !SideToMove;
    uint64_t our_king_bitboard = board.getBitboard<PieceType::KING, SideToMove>();

    uint64_t enemy_occupancy = board.generateOccupancy<! SideToMove>();
    uint64_t our_occupancy   = board.generateOccupancy<SideToMove>();

    // --- Pawn Checks ---
    uint64_t potential_checkers_pawns = generatePawnsAttacks<SideToMove>(our_king_bitboard);
    restriction_context.checkers |=
        potential_checkers_pawns & board.getBitboard<PieceType::PAWN, ! SideToMove>();

    // --- Knight Checks ---
    // Generate knight attacks from our king and then check if there is
    // opponent knight present.
    restriction_context.checkers |= generateKnightsAttacks(our_king_bitboard) &
                                    board.getBitboard<PieceType::KNIGHT, ! SideToMove>();

    // --- Ray Pieces Checks ---
    uint64_t enemy_bishops_queens = board.getDiagonalSliders<! SideToMove>();

    uint64_t diag_mask = SQUARE_TO_DIAGONAL_BITBOARD[std::to_underlying(board.getOurKingSquare())];
    uint64_t occ_diag  = board.getAllOccupancy() & diag_mask;

    processSlidingPieceChecks<SlidingPieceType::DIAGONAL>(
        our_king_bitboard, enemy_bishops_queens, occ_diag, diag_mask, our_occupancy,
        generateUpperLeftDiagonalAttacks, restriction_context);

    processSlidingPieceChecks<SlidingPieceType::DIAGONAL>(
        our_king_bitboard, enemy_bishops_queens, occ_diag, diag_mask, our_occupancy,
        generateBottomRightDiagonalAttacks, restriction_context);

    uint64_t counter_diag_mask =
        SQUARE_TO_COUNTER_DIAGONAL_BITBOARD[std::to_underlying(board.getOurKingSquare())];

    uint64_t occ_counter_diag = board.getAllOccupancy() & counter_diag_mask;
    processSlidingPieceChecks<SlidingPieceType::DIAGONAL>(
        our_king_bitboard, enemy_bishops_queens, occ_counter_diag, counter_diag_mask, our_occupancy,
        generateUpperRightDiagonalAttacks, restriction_context);
    processSlidingPieceChecks<SlidingPieceType::DIAGONAL>(
        our_king_bitboard, enemy_bishops_queens, occ_counter_diag, counter_diag_mask, our_occupancy,
        generateBottomLeftDiagonalAttacks, restriction_context);

    uint64_t enemy_rook_queens = board.getBitboard<PieceType::ROOK, ! SideToMove>() |
                                 board.getBitboard<PieceType::QUEEN, ! SideToMove>();
    uint64_t file_mask =
        FILE_BITBOARDS[std::to_underlying(convert::toFile(board.getOurKingSquare()))];
    uint64_t occ_file = board.getAllOccupancy() & file_mask;

    processSlidingPieceChecks<SlidingPieceType::HORIZONTAL_VERTICAL>(
        our_king_bitboard, enemy_rook_queens, occ_file, file_mask, our_occupancy,
        generateBottomAttacks, restriction_context);
    processSlidingPieceChecks<SlidingPieceType::HORIZONTAL_VERTICAL>(
        our_king_bitboard, enemy_rook_queens, occ_file, file_mask, our_occupancy,
        generateTopAttacks, restriction_context);
    uint64_t rank_mask =
        RANK_BITBOARDS[std::to_underlying(convert::toRank(board.getOurKingSquare()))];
    uint64_t occ_rank = board.getAllOccupancy() & rank_mask;

    processSlidingPieceChecks<SlidingPieceType::HORIZONTAL_VERTICAL>(
        our_king_bitboard, enemy_rook_queens, occ_rank, rank_mask, our_occupancy,
        generateLeftAttacks, restriction_context);
    processSlidingPieceChecks<SlidingPieceType::HORIZONTAL_VERTICAL>(
        our_king_bitboard, enemy_rook_queens, occ_rank, rank_mask, our_occupancy,
        generateRightAttacks, restriction_context);

    restriction_context.check_count = std::popcount(restriction_context.checkers);
    restriction_context.updateCheckmask();
}

inline bool isCheckedHorizontallyOnRank(uint64_t our_king_bitboard,
                                        uint64_t occupancy,
                                        uint64_t enemy_horizontal_sliders,
                                        uint64_t rank_bitboard) {
    uint64_t occupancy_on_rank = occupancy & rank_bitboard;

    uint64_t left_attacks_from_king =
        generateLeftAttacks(our_king_bitboard, occupancy_on_rank, occupancy_on_rank);
    uint64_t right_attacks_from_king =
        generateRightAttacks(our_king_bitboard, occupancy_on_rank, rank_bitboard);

    return ((left_attacks_from_king | right_attacks_from_king) & enemy_horizontal_sliders) !=
           EMPTY_BITBOARD;
}

} // namespace bitcrusher

#endif // BITCRUSHER_CHECKS_PINS_DETECTION_HPP