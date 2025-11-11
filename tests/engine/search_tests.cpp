#include "move_sink.hpp"
#include "search.hpp"
#include "search_manager.hpp"
#include <gtest/gtest.h>
#include <string>

using bitcrusher::Move;
using bitcrusher::SearchManager;

TEST(searchTests, SearchShouldRespectMaxNodeLimit) {
    SearchManager search_manager{};
    uint64_t      node_count{0};
    search_manager.setOnSearchFinished(
        [&search_manager, &node_count]() { node_count = search_manager.getNodeCount(); });
    search_manager.setPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    bitcrusher::SearchParameters params;
    params.max_nodes             = 10;
    params.use_quiescence_search = false;

    search_manager.startSearch<bitcrusher::FastMoveSink>(params);
    search_manager.waitUntilSearchFinished();

    EXPECT_EQ(node_count, 10);
}

TEST(searchTests, SearchShouldOnlyUseOnlySearchMovesIfSpecified) {
    SearchManager search_manager{};
    uint64_t      node_count = 0;
    std::string   best_move;

    search_manager.setOnSearchFinished([&search_manager, &node_count, &best_move]() {
        best_move  = search_manager.bestMoveUci();
        node_count = search_manager.getNodeCount();
    });
    search_manager.setPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    bitcrusher::SearchParameters params;
    params.addSearchMove("e2e4");
    params.max_ply               = 1;
    params.use_quiescence_search = false;

    search_manager.startSearch<bitcrusher::FastMoveSink>(params);
    search_manager.waitUntilSearchFinished();

    EXPECT_EQ(node_count, 1);
    EXPECT_EQ(best_move, "e2e4"); // The only move in search moves.
}

TEST(searchTests, SearchShouldRestrictToMultipleSearchMoves) {
    SearchManager search_manager{};
    uint64_t      node_count = 0;
    std::string   best_move;
    search_manager.setOnSearchFinished(
        [&search_manager, &best_move]() { best_move = search_manager.bestMoveUci(); });
    search_manager.setPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    bitcrusher::SearchParameters params;
    params.addSearchMove("e2e4");
    params.addSearchMove("d2d4");
    params.addSearchMove("c2c4");
    params.max_ply               = 1;
    params.use_quiescence_search = false;

    search_manager.startSearch<bitcrusher::FastMoveSink>(params);
    search_manager.waitUntilSearchFinished();

    EXPECT_TRUE(best_move == "e2e4" || best_move == "d2d4" || best_move == "c2c4");
}

TEST(searchTests, SearchMovesWithInvalidMoveShouldNotSearch) {
    SearchManager search_manager{};
    std::string   best_move;
    search_manager.setOnSearchFinished(
        [&search_manager, &best_move]() { best_move = search_manager.bestMoveUci(); });
    search_manager.setPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    bitcrusher::SearchParameters params;
    params.addSearchMove("e2e5");
    params.max_ply               = 1;
    params.use_quiescence_search = false;

    search_manager.startSearch<bitcrusher::FastMoveSink>(params);
    search_manager.waitUntilSearchFinished();

    EXPECT_TRUE(best_move == toUci(Move::none()));
}

TEST(searchTests, SearchMovesProduceConsistentEvaluationAsUnrestricted) {
    // Unrestricted search.
    SearchManager unrestricted_search_manager{};
    std::string   unrestricted_best_move;
    std::string   unrestricted_search_eval{};
    unrestricted_search_manager.setOnSearchFinished(
        [&unrestricted_search_manager, &unrestricted_search_eval, &unrestricted_best_move]() {
            unrestricted_best_move   = unrestricted_search_manager.bestMoveUci();
            unrestricted_search_eval = unrestricted_search_manager.getScore();
        });
    unrestricted_search_manager.setPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    bitcrusher::SearchParameters params_unrestricted;
    params_unrestricted.max_ply               = 3;
    params_unrestricted.use_quiescence_search = false;

    unrestricted_search_manager.startSearch<bitcrusher::FastMoveSink>(params_unrestricted);
    unrestricted_search_manager.waitUntilSearchFinished();

    // Restricted search.
    SearchManager restricted_search_manager{};
    std::string   restricted_search_eval{};

    restricted_search_manager.setOnSearchFinished(
        [&restricted_search_manager, &restricted_search_eval]() {
            restricted_search_eval = restricted_search_manager.getScore();
        });
    restricted_search_manager.setPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    bitcrusher::SearchParameters params_restricted{};
    params_restricted.max_ply               = 3;
    params_restricted.use_quiescence_search = false;
    params_restricted.addSearchMove(unrestricted_best_move);

    restricted_search_manager.startSearch<bitcrusher::FastMoveSink>(params_restricted);
    restricted_search_manager.waitUntilSearchFinished();

    EXPECT_EQ(unrestricted_search_eval, restricted_search_eval);
}

TEST(searchTests, MateIn1) {
    SearchManager search_manager{};
    std::string   best_move;
    std::string   eval;
    search_manager.setOnSearchFinished([&search_manager, &best_move, &eval]() {
        best_move = search_manager.bestMoveUci();
        eval      = search_manager.getScore();
    });
    search_manager.setPos("1rb5/4r3/3p1npb/3kp1P1/1P3P1P/5nR1/2Q1BK2/bN4NR w - - 3 61");
    bitcrusher::SearchParameters params;
    params.max_ply               = 1;
    params.use_quiescence_search = false;

    search_manager.startSearch<bitcrusher::FastMoveSink>(params);
    search_manager.waitUntilSearchFinished();

    EXPECT_EQ(best_move, "c2c4");
    EXPECT_EQ(eval, "mate 1");
}

TEST(searchTests, GettingMatedIn1) {
    SearchManager search_manager{};
    std::string   best_move;
    std::string   eval;
    search_manager.setOnSearchFinished([&search_manager, &best_move, &eval]() {
        best_move = search_manager.bestMoveUci();
        eval      = search_manager.getScore();
    });
    search_manager.setPos("r1bq2r1/b4pk1/p1pp1p1Q/1p2pP2/1P2P1PB/3P4/1PP3P1/R3K2R b - - 1 1");
    bitcrusher::SearchParameters params;
    params.max_ply               = 2;
    params.use_quiescence_search = false;

    search_manager.startSearch<bitcrusher::FastMoveSink>(params);
    search_manager.waitUntilSearchFinished();

    EXPECT_EQ(best_move, "g7h6");
    EXPECT_EQ(eval, "mate -1");
}
