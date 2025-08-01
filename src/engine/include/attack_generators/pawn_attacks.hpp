#ifndef BITCRUSHER_PAWN_ATTACKS_HPP
#define BITCRUSHER_PAWN_ATTACKS_HPP

#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"
#include "concepts.hpp"
#include "file_rank_bitboards.hpp"

namespace bitcrusher {

inline constexpr uint64_t PROMOTION_RANKS_MASK =
    RANK_BITBOARDS[std::to_underlying(Rank::R_1)] | RANK_BITBOARDS[std::to_underlying(Rank::R_8)];
inline constexpr uint64_t NON_PROMOTION_RANKS_MASK = ~PROMOTION_RANKS_MASK;

// Returns pawn push direction at compile time
template <Color Side> [[nodiscard]] consteval Direction getPawnPushDirection() {
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
    return offset::shiftBitboardNoWrap<getPawnPushDirection<Side>(), PawnAttackDirection>(pawns);
}

template <Color Side> consteval int pawnPushOffset() {
    return offset::calculateOffset<getPawnPushDirection<Side>()>();
}

template <Color Side> consteval int pawnDoublePushOffset() {
    return offset::calculateOffset<offset::RepeatedDirection{getPawnPushDirection<Side>(), 2}>();
}

// Returns only those pawns that are still on their starting rank so that a double-push is allowed.
template <Color Side>
[[nodiscard]] constexpr uint64_t getPawnsOnStartRank(uint64_t pawns) noexcept {
    constexpr uint64_t PAWN_START_RANK = Side == Color::WHITE
                                             ? RANK_BITBOARDS[std::to_underlying(Rank::R_2)]
                                             : RANK_BITBOARDS[std::to_underlying(Rank::R_7)];
    return pawns & PAWN_START_RANK;
}

template <Color Side> constexpr uint64_t generatePawnsAttacks(uint64_t pawns) {
    return generatePawnSingleSideAttacks<Side, Direction::LEFT>(pawns) |
           generatePawnSingleSideAttacks<Side, Direction::RIGHT>(pawns);
}

// Generates a single pawn push bitboard for each pawn of a given side.
template <Color Side> constexpr uint64_t pawnSinglePush(uint64_t pawns_bitboard) {
    return offset::shiftBitboardNoWrap<getPawnPushDirection<Side>()>(pawns_bitboard);
}

// Generates a double pawn push bitboard for each pawn of a given side.
template <Color Side> consteval uint64_t pawnDoublePush(uint64_t pawns_bitboard) {
    return offset::shiftBitboardNoWrap<offset::RepeatedDirection{getPawnPushDirection<Side>(), 2}>(
        pawns_bitboard);
}

template <Color Side> consteval uint64_t getEnPassantRankMask() {
    if constexpr (Side == Color::WHITE) {
        return RANK_BITBOARDS[std::to_underlying(Rank::R_5)];
    } else { // (Side == Color::BLACK)
        return RANK_BITBOARDS[std::to_underlying(Rank::R_4)];
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_PAWN_ATTACKS_HPP