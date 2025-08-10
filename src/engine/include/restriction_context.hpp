#ifndef BITCRUSHER_RESTRICTION_CONTEXT_HPP
#define BITCRUSHER_RESTRICTION_CONTEXT_HPP

#include "attack_generators/diagonal_slider_attacks.hpp"
#include "attack_generators/horizontal_vertical_slider_attacks.hpp"
#include "attack_generators/knight_attacks.hpp"
#include "attack_generators/pawn_attacks.hpp"
#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "diagonal_bitboards.hpp"
#include <cstdint>

namespace bitcrusher {

struct RestrictionContext {
    uint64_t checkers{EMPTY_BITBOARD};    // Pieces giving check to our king
    uint64_t check_block{EMPTY_BITBOARD}; // Rays between checking pieces and our king

    uint64_t checkmask{EMPTY_BITBOARD};
    // Pieces that are part of the pinmask can only move along pinmask
    uint64_t pinmask_diagonal{EMPTY_BITBOARD};
    uint64_t pinmask_horizontal_vertical{EMPTY_BITBOARD};
    uint8_t  check_count{0};

    void reset() {
        check_block                 = EMPTY_BITBOARD;
        checkers                    = EMPTY_BITBOARD;
        pinmask_diagonal            = EMPTY_BITBOARD;
        pinmask_horizontal_vertical = EMPTY_BITBOARD;
        checkmask                   = FULL_BITBOARD;
        check_count                 = 0;
    }

    void updateCheckmask() noexcept {
        checkmask = (check_count > 0) ? (checkers | check_block) : FULL_BITBOARD;
    }

    [[nodiscard]] constexpr std::uint64_t nonRestricted(uint64_t bb) const noexcept {
        return bb & ~(pinmask_diagonal | pinmask_horizontal_vertical);
    }

    bool operator==(const RestrictionContext&) const = default;
};

template <SlidingPieceType SlidingPieceT, typename AttackGenerator>
void processSlidingPieceChecks(std::uint64_t       king_bitboard,
                               std::uint64_t       enemy_sliders,
                               std::uint64_t       line_occupancy,
                               std::uint64_t       line_mask,
                               std::uint64_t       our_occupancy,
                               AttackGenerator     generate_attacks,
                               RestrictionContext& restriction_context) {
    const std::uint64_t line_attacks = generate_attacks(king_bitboard, line_occupancy, line_mask);
    const std::uint64_t slider_checkers = line_attacks & enemy_sliders;
    const std::uint64_t potential_pins  = line_attacks & our_occupancy;
    if (slider_checkers != EMPTY_BITBOARD) {
        restriction_context.checkers |= slider_checkers;
        restriction_context.check_block |= (line_attacks ^ slider_checkers);
    } else if (potential_pins != EMPTY_BITBOARD) {
        const std::uint64_t beyond_ray =
            generate_attacks(potential_pins, line_occupancy, line_mask);
        if (beyond_ray & enemy_sliders) {
            if constexpr (SlidingPieceT == SlidingPieceType::DIAGONAL) {
                restriction_context.pinmask_diagonal |= line_attacks | beyond_ray;
            } else { // SlidingPieceT == SlidingPieceType::HORIZONTAL_VERTICAL
                restriction_context.pinmask_horizontal_vertical |= line_attacks | beyond_ray;
            }
        }
    }
}

template <Color SideToMove,
          typename AttackGenerator1,
          typename AttackGenerator2,
          SlidingPieceType SlidingPieceT>
void processSlidingPiece(BoardState&         board,
                         RestrictionContext& restriction_context,
                         std::uint64_t       enemy_sliders,
                         AttackGenerator1    generate_attacks1,
                         AttackGenerator1    generate_attacks2) {
    std::uint64_t our_king_bitboard = board.getBitboard<PieceType::KING, SideToMove>();
    Square        our_king_square   = utils::getFirstSetSquare(our_king_bitboard);
    std::uint64_t diag_mask = SQUARE_TO_DIAGONAL_BITBOARD[std::to_underlying(our_king_square)];
    std::uint64_t occ_diag  = board.getAllOccupancy() & diag_mask;

    std::uint64_t our_occupancy = board.getOwnOccupancy<SideToMove>();
    processSlidingPieceChecks<generate_attacks1, SlidingPieceT>(
        our_king_bitboard, enemy_sliders, occ_diag, diag_mask, our_occupancy, generate_attacks1,
        restriction_context);

    processSlidingPieceChecks<generate_attacks2, SlidingPieceT>(
        our_king_bitboard, enemy_sliders, occ_diag, diag_mask, our_occupancy, generate_attacks2,
        restriction_context);
}

template <Color Side>
inline void updateRestrictionContext(const BoardState&   board,
                                     RestrictionContext& restriction_context) {
    restriction_context.reset();
    std::uint64_t our_king_bitboard = board.getBitboard<PieceType::KING, Side>();

    std::uint64_t our_occupancy = board.getOwnOccupancy<Side>();

    // --- Pawn Checks ---
    std::uint64_t potential_checkers_pawns = generatePawnsAttacks<Side>(our_king_bitboard);
    restriction_context.checkers |=
        potential_checkers_pawns & board.getBitboard<PieceType::PAWN, ! Side>();

    // --- Knight Checks ---
    // Generate knight attacks from our king and then check if there is
    // opponent knight present.
    restriction_context.checkers |=
        generateKnightsAttacks(our_king_bitboard) & board.getBitboard<PieceType::KNIGHT, ! Side>();

    // --- Ray Pieces Checks ---
    std::uint64_t enemy_bishops_queens = board.getDiagonalSliders<! Side>();
    Square our_king_square  = utils::getFirstSetSquare(board.getBitboard<PieceType::KING, Side>());
    std::uint64_t diag_mask = SQUARE_TO_DIAGONAL_BITBOARD[std::to_underlying(our_king_square)];
    std::uint64_t occ_diag  = board.getAllOccupancy() & diag_mask;

    processSlidingPieceChecks<SlidingPieceType::DIAGONAL>(
        our_king_bitboard, enemy_bishops_queens, occ_diag, diag_mask, our_occupancy,
        generateUpperLeftDiagonalAttacks, restriction_context);

    processSlidingPieceChecks<SlidingPieceType::DIAGONAL>(
        our_king_bitboard, enemy_bishops_queens, occ_diag, diag_mask, our_occupancy,
        generateBottomRightDiagonalAttacks, restriction_context);

    std::uint64_t counter_diag_mask =
        SQUARE_TO_COUNTER_DIAGONAL_BITBOARD[std::to_underlying(our_king_square)];

    std::uint64_t occ_counter_diag = board.getAllOccupancy() & counter_diag_mask;
    processSlidingPieceChecks<SlidingPieceType::DIAGONAL>(
        our_king_bitboard, enemy_bishops_queens, occ_counter_diag, counter_diag_mask, our_occupancy,
        generateUpperRightDiagonalAttacks, restriction_context);

    processSlidingPieceChecks<SlidingPieceType::DIAGONAL>(
        our_king_bitboard, enemy_bishops_queens, occ_counter_diag, counter_diag_mask, our_occupancy,
        generateBottomLeftDiagonalAttacks, restriction_context);

    std::uint64_t enemy_rook_queens = board.getBitboard<PieceType::ROOK, ! Side>() |
                                      board.getBitboard<PieceType::QUEEN, ! Side>();
    std::uint64_t file_mask = FILE_BITBOARDS[std::to_underlying(convert::toFile(our_king_square))];
    std::uint64_t occ_file  = board.getAllOccupancy() & file_mask;

    processSlidingPieceChecks<SlidingPieceType::HORIZONTAL_VERTICAL>(
        our_king_bitboard, enemy_rook_queens, occ_file, file_mask, our_occupancy,
        generateBottomAttacks, restriction_context);

    processSlidingPieceChecks<SlidingPieceType::HORIZONTAL_VERTICAL>(
        our_king_bitboard, enemy_rook_queens, occ_file, file_mask, our_occupancy,
        generateTopAttacks, restriction_context);
    std::uint64_t rank_mask = RANK_BITBOARDS[std::to_underlying(convert::toRank(our_king_square))];
    std::uint64_t occ_rank  = board.getAllOccupancy() & rank_mask;

    processSlidingPieceChecks<SlidingPieceType::HORIZONTAL_VERTICAL>(
        our_king_bitboard, enemy_rook_queens, occ_rank, rank_mask, our_occupancy,
        generateLeftAttacks, restriction_context);
    processSlidingPieceChecks<SlidingPieceType::HORIZONTAL_VERTICAL>(
        our_king_bitboard, enemy_rook_queens, occ_rank, rank_mask, our_occupancy,
        generateRightAttacks, restriction_context);

    restriction_context.check_count = std::popcount(restriction_context.checkers);
    restriction_context.updateCheckmask();
}

// Helper function to determine if we are in check after making en passant
inline bool isCheckedHorizontallyOnRank(std::uint64_t our_king_bitboard,
                                        std::uint64_t occupancy,
                                        std::uint64_t enemy_horizontal_sliders,
                                        std::uint64_t rank_bitboard) {
    std::uint64_t occupancy_on_rank = occupancy & rank_bitboard;

    std::uint64_t left_attacks_from_king =
        generateLeftAttacks(our_king_bitboard, occupancy_on_rank, occupancy_on_rank);
    std::uint64_t right_attacks_from_king =
        generateRightAttacks(our_king_bitboard, occupancy_on_rank, rank_bitboard);

    return ((left_attacks_from_king | right_attacks_from_king) & enemy_horizontal_sliders) !=
           EMPTY_BITBOARD;
}

} // namespace bitcrusher

#endif // BITCRUSHER_RESTRICTION_CONTEXT_HPP