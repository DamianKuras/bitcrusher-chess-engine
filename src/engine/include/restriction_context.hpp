#ifndef BITCRUSHER_RESTRICTION_CONTEXT_HPP
#define BITCRUSHER_RESTRICTION_CONTEXT_HPP

#include "bitboard_enums.hpp"

namespace bitcrusher {

struct RestrictionContext {
    uint64_t checkers{EMPTY_BITBOARD};    // pieces giving check to our king
    uint64_t check_block{EMPTY_BITBOARD}; // ray between checking piece and our king

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
};

} // namespace bitcrusher

#endif // BITCRUSHER_RESTRICTION_CONTEXT_HPP