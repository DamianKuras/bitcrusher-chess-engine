#ifndef BITCRUSHER_HEURISTICS_SEARCH_CONFIG_HPP
#define BITCRUSHER_HEURISTICS_SEARCH_CONFIG_HPP

namespace bitcrusher {

struct TTMoveOrderingConfig {
    bool enabled = false;
};

struct MVVLVAConfig {
    bool enabled = false;
};

struct QuiescenceConfig {
    bool enabled = false;
};

struct SearchConfig {
    TTMoveOrderingConfig tt_move_ordering{};
    MVVLVAConfig         mvv_lva{};
    QuiescenceConfig     quiescence{};
};

// Matches the current engine behaviour.
inline constexpr SearchConfig DEFAULT_CONFIG{
    .tt_move_ordering = {.enabled = true},
    .mvv_lva          = {.enabled = true},
    .quiescence       = {.enabled = true},
};

// Used when SearchParameters::use_quiescence_search is false.
inline constexpr SearchConfig NO_QUIESCENCE_CONFIG{
    .tt_move_ordering = {.enabled = true},
    .mvv_lva          = {.enabled = true},
};

} // namespace bitcrusher

#endif // BITCRUSHER_HEURISTICS_SEARCH_CONFIG_HPP
