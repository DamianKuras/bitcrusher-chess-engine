#include "move_sink.hpp"
#include "search.hpp"
#include "search_manager.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <unordered_set>

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

TEST(searchTests, PonderingSuspendsSearchUntilPonderHit) {
    SearchManager     search_manager{};
    std::string       best_move;
    std::atomic<bool> search_finished{false};

    search_manager.setOnSearchFinished([&search_manager, &best_move, &search_finished]() {
        best_move       = search_manager.bestMoveUci();
        search_finished = true;
    });

    // Setting up a basic position
    search_manager.setPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    bitcrusher::SearchParameters params;
    params.max_ply = 3; // Let it search just 3 plys so it finishes the logical "search" quickly.
    params.use_quiescence_search = false;
    params.ponder                = true; // This should cause the manager to pause outputting.

    search_manager.startSearch<bitcrusher::FastMoveSink>(params);

    // Wait for a small amount of time to ensure the logical search has completed,
    // but the output shouldn't have been emitted yet because of pondering.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    EXPECT_FALSE(search_finished.load()); // Should not have emitted yet.

    // Now trigger the ponder hit
    search_manager.ponderHit();
    search_manager.waitUntilSearchFinished(); // Wait for actual completion.

    EXPECT_TRUE(search_finished.load());
    EXPECT_NE(best_move, "");
}

TEST(searchTests, SearchOutputsLegalFallbackMoveWhenAbortedInstantly) {
    SearchManager search_manager{};
    std::string   best_move;

    search_manager.setOnSearchFinished(
        [&search_manager, &best_move]() { best_move = search_manager.bestMoveUci(); });

    // 1. Search a normal position so best_move_ gets populated with some move.
    search_manager.setPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    bitcrusher::SearchParameters params;
    params.max_ply = 3;
    search_manager.startSearch<bitcrusher::FastMoveSink>(params);
    search_manager.waitUntilSearchFinished();

    std::string previous_best_move = best_move;

    // 2. Set a completely different position where the previous move is ILLEGAL.
    search_manager.setPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    bitcrusher::SearchParameters instant_abort_params;
    instant_abort_params.use_quiescence_search = false;

    // PauseAfterRootSort=true makes the search sleep 500ms after generating and
    // sorting root moves (setting root_best_move), so stopSearch() is guaranteed
    // to fire while the search is paused — deterministic, no timing guesswork.
    search_manager.startSearch<bitcrusher::FastMoveSink, true>(instant_abort_params);
    search_manager.stopSearch();
    search_manager.waitUntilSearchFinished();

    // The engine MUST NOT emit the move from the previous search (or a8a8)!
    EXPECT_NE(best_move, previous_best_move);
    EXPECT_NE(best_move, "a8a8");

    // Verify it emits a legal move for the current position instead of a stale one.
    bool is_legal =
        (best_move == "e2e4" || best_move == "d2d4" || best_move == "e2e3" || best_move == "d2d3" ||
         best_move == "g1f3" || best_move == "b1c3" || best_move == "g1h3" || best_move == "b1a3" ||
         best_move == "a2a3" || best_move == "a2a4" || best_move == "b2b3" || best_move == "b2b4" ||
         best_move == "c2c3" || best_move == "c2c4" || best_move == "f2f3" || best_move == "f2f4" ||
         best_move == "g2g3" || best_move == "g2g4" || best_move == "h2h3" || best_move == "h2h4");
    EXPECT_TRUE(is_legal);
}

TEST(searchTests, PrincipalVariationShouldBeCycleFree) {
    // Exact position from SPRT where "Warning; Illegal pv move c5c6 from Bitcrusher_dev"
    // appeared at depth 21+. TT entries for the K+R vs K mating sequence cycled back,
    // causing getPrincipalVariation to emit an illegal move.
    // FEN derived from: startpos + 162 game moves (python-chess verified).
    SearchManager search_manager{};
    std::string   pv;

    search_manager.setOnSearchFinished(
        [&search_manager, &pv]() { pv = search_manager.getPrincipalVariation(80); });

    const std::string_view fen = "1k6/8/2RK4/8/8/8/8/8 w - - 19 92";
    search_manager.setPos(fen);

    bitcrusher::SearchParameters params;
    params.max_ply               = 80; // depth 40; bug appeared at depth 21+
    params.use_quiescence_search = false;

    search_manager.startSearch<bitcrusher::FastMoveSink>(params);
    search_manager.waitUntilSearchFinished();

    ASSERT_FALSE(pv.empty());

    // Walk PV moves and verify no position hash appears twice.
    bitcrusher::BoardState pv_board;
    bitcrusher::parseFEN(fen, pv_board);
    bitcrusher::MoveProcessor    pv_mp;
    std::unordered_set<uint64_t> visited;
    visited.insert(pv_board.getZobristHash());

    std::istringstream iss(pv);
    std::string        move_uci;
    while (iss >> move_uci) {
        bitcrusher::Move move = bitcrusher::moveFromUci(move_uci, pv_board);
        pv_mp.applyMove(pv_board, move);
        const bool is_new = visited.insert(pv_board.getZobristHash()).second;
        EXPECT_TRUE(is_new) << "PV contains a cycle at move: " << move_uci;
        if (! is_new) {
            break;
        }
    }
}
