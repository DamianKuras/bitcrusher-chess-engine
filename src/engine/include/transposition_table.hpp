#ifndef BITCRUSHER_TRANSPOSITION_TABLE_HPP
#define BITCRUSHER_TRANSPOSITION_TABLE_HPP

#include "move.hpp"
#include <climits>
#include <mutex>
#include <vector>

namespace bitcrusher {

inline constexpr int NOT_FOUND_IN_TRANSPOSITION_TABLE = INT_MIN;

inline constexpr uint32_t BUCKET_COUNT = 1024;

inline constexpr uint32_t MIN_TABLE_SIZE = 1024;

enum class TranspositionTableEvaluationType : std::uint8_t { EXACT_VALUE, AT_BEST, AT_LEAST };

struct TranspositionTableEntry {
    uint64_t                         key{0ULL};
    int                              depth{-1}; // -1 indicates empty slot
    int                              value{NOT_FOUND_IN_TRANSPOSITION_TABLE};
    TranspositionTableEvaluationType evaluation_type{TranspositionTableEvaluationType::EXACT_VALUE};
    Move                             best_move;
};

class TranspositionTable {

    std::vector<TranspositionTableEntry> table_;
    std::array<std::mutex, BUCKET_COUNT> locks_; // One mutex per bucket

    uint32_t size_{};

    [[nodiscard]] std::mutex& getLock(uint64_t key) { return locks_[key % locks_.size()]; }

    [[nodiscard]] uint32_t indexForKey(uint64_t key) const { return key % size_; }

public:
    TranspositionTable() { setSize(MIN_TABLE_SIZE); }

    explicit TranspositionTable(uint32_t size) { setSize(size); }

    void store(int                              depth,
               int                              eval,
               TranspositionTableEvaluationType eval_type,
               uint64_t                         key,
               Move                             best_move) {
        uint32_t                    index = indexForKey(key);
        std::lock_guard<std::mutex> lock(getLock(key));

        TranspositionTableEntry& entry = table_[index];
        if (depth >= entry.depth) { // Replace if deeper or equal depth
            entry = TranspositionTableEntry{.key             = key,
                                            .depth           = depth,
                                            .value           = eval,
                                            .evaluation_type = eval_type,
                                            .best_move       = best_move};
        }
    }

    int getPositionEvaluation(int depth, int alpha, int beta, uint64_t key) {

        std::lock_guard<std::mutex> lock(getLock(key));
        TranspositionTableEntry&    entry = table_[indexForKey(key)];
        if (entry.key != key || entry.depth < depth) {
            return NOT_FOUND_IN_TRANSPOSITION_TABLE;
        }
        if (entry.evaluation_type == TranspositionTableEvaluationType::EXACT_VALUE) {
            return entry.value;
        }
        if (entry.evaluation_type == TranspositionTableEvaluationType::AT_BEST &&
            entry.value <= alpha) {
            return alpha;
        }
        if (entry.evaluation_type == TranspositionTableEvaluationType::AT_LEAST &&
            entry.value >= beta) {
            return beta;
        }
        return NOT_FOUND_IN_TRANSPOSITION_TABLE;
    }

    [[nodiscard]] TranspositionTableEntry getEntry(uint64_t key) const {
        return table_[key % size_];
    }

    void setSize(uint32_t size) {
        size_ = size;
        table_.resize(size);
    }
};

} // namespace bitcrusher

#endif // BITCRUSHER_TRANSPOSITION_TABLE_HPP