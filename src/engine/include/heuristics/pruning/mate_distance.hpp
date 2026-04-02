#ifndef BITCRUSHER_HEURISTICS_MATE_DISTANCE_HPP
#define BITCRUSHER_HEURISTICS_MATE_DISTANCE_HPP

#include <optional>

namespace bitcrusher::heuristics {

// Prune when the best possible mate at this distance cannot improve alpha/beta.
// Modifies alpha and beta in place. Returns a score to propagate immediately
// if pruning fires, or nullopt if the search should continue.
[[nodiscard]] constexpr std::optional<int>
applyMateDistancePruning(int checkmate_base, int ply, int& alpha, int& beta) noexcept {
    int upper = checkmate_base - ply;
    if (upper < beta) {
        beta = upper;
        if (alpha >= beta)
            return upper;
    }
    int lower = -(checkmate_base - ply);
    if (lower > alpha) {
        alpha = lower;
        if (alpha >= beta)
            return lower;
    }
    return std::nullopt;
}

} // namespace bitcrusher::heuristics

#endif // BITCRUSHER_HEURISTICS_MATE_DISTANCE_HPP
