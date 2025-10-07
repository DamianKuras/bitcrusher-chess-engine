#ifndef BITCRUSHER_TRANSPOSITION_TABLE_HPP
#define BITCRUSHER_TRANSPOSITION_TABLE_HPP

#include "move.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

namespace bitcrusher {

inline constexpr int NOT_FOUND_IN_TRANSPOSITION_TABLE = 1000000 + 1;
inline constexpr int ON_EVALUATION                    = 1000000 + 2;

inline constexpr uint32_t BUCKET_COUNT = 10 * 1024;

enum class TranspositionTableEvaluationType : std::uint8_t { EXACT_VALUE, LOWERBOUND, UPPERBOUND };

const int DEFAULT_TT_SIZE = 1 << 20;

struct TranspositionTableEntry {
    uint64_t                         key{0ULL};
    int                              depth{-1};
    int                              value{NOT_FOUND_IN_TRANSPOSITION_TABLE};
    TranspositionTableEvaluationType evaluation_type{TranspositionTableEvaluationType::EXACT_VALUE};
    Move                             best_move = Move::none();
    uint8_t                          searching_by{0};
};

class TranspositionTable {
    std::vector<TranspositionTableEntry> table_;
    std::array<std::mutex, BUCKET_COUNT> locks_; // One mutex per bucket.

    uint32_t size_{DEFAULT_TT_SIZE};

    [[nodiscard]] std::mutex& getLock(uint64_t key) { return locks_[key % BUCKET_COUNT]; }

    [[nodiscard]] uint64_t indexForKey(uint64_t key) const { return key % table_.size(); }

#ifdef DEBUG
    std::atomic<int> used_{0};
#endif

public:
    TranspositionTable() : table_(DEFAULT_TT_SIZE) {};

    void store(uint64_t key, TranspositionTableEntry entry) {
        assert(! entry.best_move.isNullMove());
        assert(entry.value != ON_EVALUATION);
        uint64_t                    index = indexForKey(key);
        std::lock_guard<std::mutex> lock(getLock(key));
        TranspositionTableEntry&    current_entry = table_[index];
        if (entry.depth >= current_entry.depth) { // Replace if closer to root or more recent.
#ifdef DEBUG
            if (current_entry.depth == -1) {
                used_.fetch_add(1);
            }
#endif
            table_[index] = entry;
        }
    }

    [[nodiscard]] TranspositionTableEntry getEntry(uint64_t key) {
        uint64_t                    index = indexForKey(key);
        std::lock_guard<std::mutex> lock(getLock(key));
        TranspositionTableEntry&    entry = table_[index];
        return entry;
    }

    void addSearched(uint64_t key) {
        uint64_t index = indexForKey(key);
        table_[index].searching_by += 1;
    }

    void removeSearched(uint64_t key) {
        uint64_t index = indexForKey(key);
        table_[index].searching_by -= 1;
    }

    bool isSearched(uint64_t key) {
        uint64_t index = indexForKey(key);
        return table_[index].searching_by > 0;
    }

    void setMBSize(size_t size) {
        constexpr std::size_t BYTES_PER_KILOBYTE     = 1024ULL;
        constexpr std::size_t KILOBYTES_PER_MEGABYTE = 1024ULL;
        size_t target_memory_bytes = size * KILOBYTES_PER_MEGABYTE * BYTES_PER_KILOBYTE;
        size_t object_size         = sizeof(TranspositionTableEntry);
        size_t num_objects         = target_memory_bytes / object_size;
        setSize(num_objects);
    }

    void setSize(size_t size) {
        size_ = size;
        table_.resize(size, TranspositionTableEntry());
        table_.shrink_to_fit();
    }

    void clear() {
        std::ranges::fill(table_, TranspositionTableEntry());
#ifdef DEBUG
        used_.store(0);
#endif
    }
#ifdef DEBUG
    double getUsedPercentage() const {
        return size_ > 0 ? (static_cast<double>(used_.load()) / static_cast<double>(size_)) * 100.0
                         : 0.0;
    }

#endif
};

} // namespace bitcrusher

#endif // BITCRUSHER_TRANSPOSITION_TABLE_HPP