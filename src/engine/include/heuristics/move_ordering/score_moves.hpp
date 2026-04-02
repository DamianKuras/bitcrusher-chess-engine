#ifndef BITCRUSHER_HEURISTICS_SCORE_MOVES_HPP
#define BITCRUSHER_HEURISTICS_SCORE_MOVES_HPP

#include "move.hpp"
#include "move_sink.hpp"
#include "search_config.hpp"
#include <utility>

namespace bitcrusher::heuristics {

[[nodiscard]] constexpr int mvvLvaPieceValue(PieceType pt) noexcept {
    switch (pt) {
    case PieceType::QUEEN:
        return 900;
    case PieceType::ROOK:
        return 500;
    case PieceType::BISHOP:
        return 330;
    case PieceType::KNIGHT:
        return 320;
    case PieceType::PAWN:
        return 100;
    default:
        return 0;
    }
}

// Score a move for ordering in the main search.
// Priority (highest first): TT move, captures (MVV-LVA), promotions, quiet.
template <SearchConfig Config>
[[nodiscard]] constexpr int scoreMoveMain(const Move& move, const Move& tt_move) noexcept {
    if constexpr (Config.tt_move_ordering.enabled) {
        if (move == tt_move)
            return 1'000'000;
    }
    if (move.isCapture()) {
        if constexpr (Config.mvv_lva.enabled) {
            return (10 * mvvLvaPieceValue(move.capturedPiece())) -
                   mvvLvaPieceValue(move.movingPiece()) + 10'000;
        }
        return 10'000;
    }
    if (move.isPromotion())
        return 5'900;
    return 0;
}

// Score a move for ordering in quiescence search (captures and promotions only).
template <SearchConfig Config>
[[nodiscard]] constexpr int scoreMoveQuiescence(const Move& move) noexcept {
    if (move.isCapture()) {
        if constexpr (Config.mvv_lva.enabled) {
            return (10 * mvvLvaPieceValue(move.capturedPiece())) -
                   mvvLvaPieceValue(move.movingPiece()) + 10'000;
        }
        return 10'000;
    }
    if (move.isPromotion())
        return 5'900;
    return 0;
}

template <MoveSink MoveSinkT> void sortMoves(MoveSinkT& sink, int* move_scores, int ply) {
    int count = sink.count[ply];
    for (int i = 0; i < count - 1; ++i) {
        int best_idx = i;
        for (int j = i + 1; j < count; ++j) {
            if (move_scores[j] > move_scores[best_idx])
                best_idx = j;
        }
        std::swap(sink.moves[ply][i], sink.moves[ply][best_idx]);
        std::swap(move_scores[i], move_scores[best_idx]);
    }
}

template <SearchConfig Config, MoveSink MoveSinkT>
void scoreAndSortQuiescence(MoveSinkT& sink, int ply) {
    int move_scores[MAX_LEGAL_MOVES];
    for (int i = 0; i < sink.count[ply]; ++i)
        move_scores[i] = scoreMoveQuiescence<Config>(sink.moves[ply][i]);
    sortMoves(sink, move_scores, ply);
}

template <SearchConfig Config, MoveSink MoveSinkT>
void scoreAndSort(MoveSinkT& sink, const Move& tt_move, int ply) {
    int move_scores[MAX_LEGAL_MOVES];
    for (int i = 0; i < sink.count[ply]; ++i)
        move_scores[i] = scoreMoveMain<Config>(sink.moves[ply][i], tt_move);
    sortMoves(sink, move_scores, ply);
}

} // namespace bitcrusher::heuristics

#endif // BITCRUSHER_HEURISTICS_SCORE_MOVES_HPP
