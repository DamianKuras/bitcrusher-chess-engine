#ifndef BITCRUSHER_PAWN_MOVES_HPP
#define BITCRUSHER_PAWN_MOVES_HPP

#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"
#include "concepts.hpp"
#include "file_rank_bitboards.hpp"
#include "move.hpp"
#include "move_generator.hpp"

namespace bitcrusher {

inline constexpr uint64_t PROMOTION_RANKS_MASK =
    RANK_BITBOARDS[std::to_underlying(Rank::R_1)] | RANK_BITBOARDS[std::to_underlying(Rank::R_8)];
inline constexpr uint64_t NON_PROMOTION_RANKS_MASK = ~PROMOTION_RANKS_MASK;


// Returns pawn push direction at compile time
template <Color Side>
[[nodiscard]] consteval Direction getPawnPushDirection() {
    if constexpr (Side == Color::WHITE) {
        return Direction::TOP;
    } else { // Side==Color::BLACK
        return Direction::BOTTOM;
    }
}

template <Color Side, Direction PawnAttackDirection>
    requires Horizontal<PawnAttackDirection>
[[nodiscard]] consteval int getPawnAttackOffset() {
    return offset::calculateOffset<getPawnPushDirection<Side>(), PawnAttackDirection>();
}

template <Color Side, Direction PawnAttackDirection>
    requires Horizontal<PawnAttackDirection>
[[nodiscard]] constexpr uint64_t generatePawnSingleSideAttacks(uint64_t pawns) {
    return offset::shiftBitboardNoWrap<getPawnPushDirection<Side>(), PawnAttackDirection>(
        pawns);
}

template <Color Side>
[[nodiscard]] constexpr uint64_t validDoublePushPawns(uint64_t pawns) noexcept {
    constexpr uint64_t PAWN_START_RANK = Side == Color::WHITE
                                        ? RANK_BITBOARDS[std::to_underlying(Rank::R_2)]
                                        : RANK_BITBOARDS[std::to_underlying(Rank::R_7)];
    return pawns & PAWN_START_RANK;
}

template <Color Side>
constexpr uint64_t generatePawnsAttacks(uint64_t pawns) {
    return generatePawnSingleSideAttacks<Side, Direction::LEFT>(pawns) |
           generatePawnSingleSideAttacks<Side, Direction::RIGHT>(pawns);
}

// Generates a single pawn push bitboard for each pawn of a given side.
template <Color Side>
consteval uint64_t pawnSinglePush(uint64_t pawns_bitboard) {
    return offset::shiftBitboardNoWrap<getPawnPushDirection<Side>()>(pawns_bitboard);
}

// Generates a double pawn push bitboard for each pawn of a given side.
template <Color Side>
consteval uint64_t pawnDoublePush(uint64_t pawns_bitboard) {
    return offset::shiftBitboardNoWrap<offset::RepeatedDirection{getPawnPushDirection<Side>(),
                                                                 2}>(pawns_bitboard);
}

template <Color Side>
consteval uint64_t getEnPassantRank() {
    if constexpr (Side == Color::WHITE) {
        return RANK_BITBOARDS[std::to_underlying(Rank::R_5)];
    } else { // (Side == Color::BLACK)
        return RANK_BITBOARDS[std::to_underlying(Rank::R_4)];
    }
}

template <Color Side, Direction PawnAttackDirection>
constexpr bool isValidEnPassant(const uint64_t pawns_side_attacks,
                                const uint64_t en_passant_bitboard,
                                const uint64_t occupancy_all,
                                const uint64_t our_king_bitboard,
                                const uint64_t enemy_horizontal_vertical_sliders) {

    constexpr uint64_t EN_PASSANT_RANK_BITBOARD = getEnPassantRank<Side>();
    // No valid pawn attack on en passant square
    if ((pawns_side_attacks & en_passant_bitboard) == EMPTY_BITBOARD) {
        return false;
    }
    // We have a valid pawn attacks on the ep square and our king is not on ep_rank
    if (our_king_bitboard & EN_PASSANT_RANK_BITBOARD) {
        return true;
    }
    // Our king is on en passant rank as well as capturing and captured pawns en_passant could
    // expose a check to the king
    uint64_t ep_from_square_bitboard =
        offset::shiftBitboardNoWrap<getOppositeDirection<getPawnPushDirection<Side>>(),
                                    getOppositeDirection<PawnAttackDirection>()>(
            en_passant_bitboard);
    uint64_t pawn_captured_by_ep_bitboard =
        offset::shiftBitboardNoWrap<PawnAttackDirection>(ep_from_square_bitboard);

    // Temporary remove both capturing and captured pawn from occupancy
    uint64_t temporary_occupancy =
        occupancy_all ^ ep_from_square_bitboard ^ pawn_captured_by_ep_bitboard;

    if (isCheckedHorizontallyOnRank(our_king_bitboard, temporary_occupancy,
                                    enemy_horizontal_vertical_sliders, EN_PASSANT_RANK_BITBOARD)) {
        return false;
    }
}

// Generates pawn attack moves for a specific attack direction
template <MoveSink Sink, Color Side, Direction PawnAttackDirection>
    requires Horizontal<PawnAttackDirection>
void generatePawnAttackMoves(uint64_t pawns_not_pinned,
                             uint64_t pawns_pinned_only_d,
                             uint64_t pinmask_d,
                             uint64_t enemy_occupancy,
                             uint64_t occupancy_all,
                             uint64_t checkmask,
                             uint64_t en_passant_bitboard,
                             uint64_t enemy_horizontal_vertical_sliders,
                             uint64_t our_king_bitboard,
                             Sink&    sink) {
    const uint64_t pawns_side_attacks =
        generatePawnsAttacks<Side, PawnAttackDirection>(pawns_not_pinned) &
        (generatePawnsAttacks<Side, PawnAttackDirection>(pawns_pinned_only_d) & pinmask_d) &
        enemy_occupancy & checkmask;

    uint64_t pawns_side_attacks_promotions     = pawns_side_attacks & PROMOTION_RANKS_MASK;
    uint64_t pawns_side_attacks_non_promotions = pawns_side_attacks & NON_PROMOTION_RANKS_MASK;

    createMovesFromBitboard<Sink, MoveType::CAPTURE>(
        sink, pawns_side_attacks_non_promotions,
        getPawnAttackOffset<Side, PawnAttackDirection>);

    createMovesFromBitboard<Sink, MoveType::PROMOTION_CAPTURE>(
        sink, pawns_side_attacks_promotions, getPawnAttackOffset<Side, PawnAttackDirection>);

    // En passant
    if (isValidEnPassant(pawns_side_attacks_non_promotions, en_passant_bitboard, occupancy_all,
                         our_king_bitboard, enemy_horizontal_vertical_sliders)) {
        createMovesFromBitboard<Sink, MoveType::EN_PASSANT>(
            sink, en_passant_bitboard, getPawnAttackOffset<Side, PawnAttackDirection>);
    }
}

template <MoveSink Sink, Color Side>
void generateLegalPawnMoves(const BoardState& board,
                            const uint64_t    pinmask_d,
                            const uint64_t    pinmask_hv,
                            const uint64_t    enemy_occupancy,
                            const uint64_t    empty_squares,
                            const uint64_t    checkmask,
                            Sink&             sink) {
    const uint64_t pawns_not_pinned =
        board.getBitboard<PieceType::PAWN, Side>() & ~(pinmask_d | pinmask_hv);
    const uint64_t pawns_pinned_only_hv =
        board.getBitboard<PieceType::PAWN, Side>() & (~pinmask_d) & pinmask_hv;
    const uint64_t pawns_pinned_only_d =
        board.getBitboard<PieceType::PAWN, Side>() & (~pinmask_hv) & pinmask_d;

    // Attacks and en_passant
    generatePawnAttackMoves<Sink, Side, Direction::LEFT>(
        pawns_not_pinned, pawns_pinned_only_d, pinmask_d, enemy_occupancy, checkmask, sink);
    generatePawnAttackMoves<Sink, Side, Direction::RIGHT>(
        pawns_not_pinned, pawns_pinned_only_d, pinmask_d, enemy_occupancy, checkmask, sink);

    // Single pawn pushes
    const uint64_t single_pawn_pushes =
        (pawnSinglePush<Side>(pawns_not_pinned) |
         (pawnSinglePush<Side>(pawns_pinned_only_hv) & pinmask_hv)) &
        empty_squares & checkmask;
    uint64_t pawn_pushes_promotions = single_pawn_pushes & PROMOTION_RANKS_MASK;

    createMovesFromBitboard<Sink, MoveType::PROMOTION>(sink, pawn_pushes_promotions,
                                                       pawnPushOffset<Side>());

    uint64_t pawn_pushes_non_promotions = single_pawn_pushes & NON_PROMOTION_RANKS_MASK;
    createMovesFromBitboard<Sink, MoveType::QUIET>(sink, pawn_pushes_non_promotions,
                                                   pawnPushOffset<Side>());
    // Double pawn pushes
    // we can't push over occupied square
    const uint64_t double_push_able_pawns_not_pinned =
        validDoublePushPawns<Side>(pawns_not_pinned);
    const uint64_t temporary_single_push_not_pinned =
        pawnSinglePush<Side>(double_push_able_pawns_not_pinned) & empty_squares;
    uint64_t double_pawn_pushes_not_pinned =
        pawnSinglePush(temporary_single_push_not_pinned) & empty_squares & checkmask;
    createMovesFromBitboard<Sink, MoveType::DOUBLE_PAWN_PUSH>(sink, double_pawn_pushes_not_pinned,
                                                              pawnDoublePushOffset<Side>());

    const uint64_t double_push_able_pawns_pinned_hv =
        validDoublePushPawns<Side>(pawns_pinned_only_hv);
    uint64_t temporary_single_push_pinned_hv =
        pawnSinglePush<Side>(double_push_able_pawns_pinned_hv) & empty_squares;
    uint64_t double_pawn_pushes_pinned_hv =
        pawnSinglePush(temporary_single_push_pinned_hv) & empty_squares & checkmask & pinmask_hv;

    createMovesFromBitboard<Sink, MoveType::DOUBLE_PAWN_PUSH>(sink, double_pawn_pushes_pinned_hv,
                                                              pawnDoublePushOffset<Side>());
}

} // namespace bitcrusher

#endif