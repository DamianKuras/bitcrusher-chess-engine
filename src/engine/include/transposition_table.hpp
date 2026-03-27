#ifndef BITCRUSHER_TRANSPOSITION_TABLE_HPP
#define BITCRUSHER_TRANSPOSITION_TABLE_HPP

#include "move.hpp"
#include <algorithm>
#include <atomic>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
// _mm_prefetch: in <intrin.h> on MSVC/ICC, in <xmmintrin.h> on GCC/Clang.
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#    include <intrin.h>
#else
#    include <xmmintrin.h>
#endif

namespace bitcrusher {

inline constexpr int NOT_FOUND_IN_TRANSPOSITION_TABLE = 1000000 + 1;
inline constexpr int ON_EVALUATION                    = 1000000 + 2;

enum class TranspositionTableEvaluationType : std::uint8_t { EXACT_VALUE, LOWERBOUND, UPPERBOUND };

const int DEFAULT_TT_SIZE = 1 << 20; // Must be a power of 2.

struct TranspositionTableEntry {
    uint64_t                         key{0ULL};
    int                              depth{-1};
    int                              value{NOT_FOUND_IN_TRANSPOSITION_TABLE};
    TranspositionTableEvaluationType evaluation_type{TranspositionTableEvaluationType::EXACT_VALUE};
    Move                             best_move = Move::none();
};

class TranspositionTable {
    std::vector<TranspositionTableEntry> table_;
    // Separate from the entry so the main entry stays trivially copyable and racy
    // reads remain safe. Atomic so add/remove/isSearched are data-race-free.
    std::vector<std::atomic<uint8_t>> searching_by_;

    uint64_t mask_{DEFAULT_TT_SIZE - 1};

    [[nodiscard]] uint64_t indexForKey(uint64_t key) const { return key & mask_; }

#ifdef DEBUG
    std::atomic<int> used_{0};
#endif

public:
    TranspositionTable() : table_(DEFAULT_TT_SIZE), searching_by_(DEFAULT_TT_SIZE) {}

    // Lock-free: racy reads/writes are intentional. The TT is a cache a torn
    // read produces an entry whose key won't match the position hash, so it is
    // silently treated as a miss.
    void store(uint64_t key, TranspositionTableEntry entry) {
        assert(entry.value != ON_EVALUATION);
        const uint64_t                 index         = indexForKey(key);
        const TranspositionTableEntry& current_entry = table_[index];

        // Replace if closer to root or more recent.
        if (entry.depth >= current_entry.depth) {
#ifdef DEBUG
            if (current_entry.depth == -1) {
                used_.fetch_add(1, std::memory_order_relaxed);
            }
#endif
            table_[index] = entry;
        }
    }

    [[nodiscard]] TranspositionTableEntry getEntry(uint64_t key) {
        return table_[indexForKey(key)];
    }

    void prefetch(uint64_t key) const {
        _mm_prefetch(reinterpret_cast<const char*>(&table_[indexForKey(key)]),
                     _MM_HINT_T0); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    }

    void addSearched(uint64_t key) {
        searching_by_[indexForKey(key)].fetch_add(1, std::memory_order_relaxed);
    }

    void removeSearched(uint64_t key) {
        searching_by_[indexForKey(key)].fetch_sub(1, std::memory_order_relaxed);
    }

    bool isSearched(uint64_t key) {
        return searching_by_[indexForKey(key)].load(std::memory_order_relaxed) > 0;
    }

    void setMBSize(size_t size) {
        constexpr std::size_t BYTES_PER_KILOBYTE     = 1024ULL;
        constexpr std::size_t KILOBYTES_PER_MEGABYTE = 1024ULL;
        const size_t target_memory_bytes = size * KILOBYTES_PER_MEGABYTE * BYTES_PER_KILOBYTE;
        const size_t object_size         = sizeof(TranspositionTableEntry);
        const size_t num_objects         = target_memory_bytes / object_size;
        setSize(num_objects);
    }

    void setSize(size_t size) {
        // Round up to next power of 2 so indexForKey can use a bitmask.
        const size_t pow2 = std::bit_ceil(size);
        mask_             = pow2 - 1;
        table_.assign(pow2, TranspositionTableEntry{});
        table_.shrink_to_fit();
        searching_by_ = std::vector<std::atomic<uint8_t>>(pow2);
    }

    void clear() {
        std::ranges::fill(table_, TranspositionTableEntry{});
        for (auto& s : searching_by_) {
            s.store(0, std::memory_order_relaxed);
        }
#ifdef DEBUG
        used_.store(0);
#endif
    }

#ifdef DEBUG
    double getUsedPercentage() const {
        size_t size = table_.size();
        return size > 0 ? (static_cast<double>(used_.load()) / static_cast<double>(size)) * 100.0
                        : 0.0;
    }
#endif
};

} // namespace bitcrusher

#endif // BITCRUSHER_TRANSPOSITION_TABLE_HPP