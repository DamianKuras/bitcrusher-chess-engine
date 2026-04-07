#include "move_sink.hpp"
#include "search_manager.hpp"
#include <cctype>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

using bitcrusher::SearchManager;

// Each entry from the UCI-format ERET EPD file.
struct EretTestCase {
    std::string              name;        // ERET id, used as GTest parameter name
    std::string              fen;         // 6-field FEN ("... 0 1" appended)
    std::vector<std::string> best_moves;  // non-empty when "bm" is present
    std::vector<std::string> avoid_moves; // non-empty when "am" is present
};

namespace {

// Parse one line of the UCI-format EPD file produced by convert_epd_san_to_uci.py.
// Format: <pos> <side> <castling> <ep> [bm <uci>...;] [am <uci>...;] [id "<name>";]
EretTestCase parseEpdLine(const std::string& line) {
    EretTestCase       tc;
    std::istringstream iss(line);
    std::string        token;
    int                fen_fields = 0;
    std::string        fen_raw;

    // Collect the 4 FEN fields.
    while (fen_fields < 4 && iss >> token) {
        if (! fen_raw.empty()) {
            fen_raw += ' ';
        }
        fen_raw += token;
        ++fen_fields;
    }
    tc.fen = fen_raw + " 0 1"; // append halfmove/fullmove so parseFEN gets 6 fields

    // Parse operations.
    std::string current_op;
    while (iss >> token) {
        if (token == "bm" || token == "am" || token == "id") {
            current_op = token;
            continue;
        }
        if (current_op == "bm") {
            std::string move = token;
            if (! move.empty() && move.back() == ';') {
                move.pop_back();
                if (! move.empty()) {
                    tc.best_moves.push_back(move);
                }
                current_op.clear();
            } else {
                tc.best_moves.push_back(move);
            }
        } else if (current_op == "am") {
            std::string move = token;
            if (! move.empty() && move.back() == ';') {
                move.pop_back();
                if (! move.empty()) {
                    tc.avoid_moves.push_back(move);
                }
                current_op.clear();
            } else {
                tc.avoid_moves.push_back(move);
            }
        } else if (current_op == "id") {
            // id "ERET 001 - Relief"; -- collect tokens until ';'
            tc.name += (tc.name.empty() ? "" : " ") + token;
            if (! token.empty() && token.back() == ';') {
                tc.name.pop_back(); // remove trailing ';'
                // Strip surrounding quotes.
                if (tc.name.size() >= 2 && tc.name.front() == '"') {
                    tc.name = tc.name.substr(1, tc.name.size() - 2);
                }
                current_op.clear();
            }
        }
    }

    // Sanitize name for use as a GTest parameter name (only alphanumerics and underscores).
    for (char& c : tc.name) {
        if (! std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
            c = '_';
        }
    }
    // Append _slow so --gtest_filter=-*slow skips these in run_tests.bat.
    tc.name += "_slow";

    return tc;
}

std::vector<EretTestCase> loadEretTestCases(const std::string& path) {
    std::ifstream             file(path);
    std::vector<EretTestCase> cases;
    std::string               line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        EretTestCase tc = parseEpdLine(line);
        if (tc.best_moves.empty() && tc.avoid_moves.empty()) {
            continue; // skip entries with no move directive
        }
        cases.push_back(std::move(tc));
    }
    return cases;
}

} // namespace

class EretSearchTest : public ::testing::TestWithParam<EretTestCase> {};

TEST_P(EretSearchTest, FindsBestMove) {
    const EretTestCase& tc = GetParam();

    SearchManager search_manager{};
    std::string   best_move;

    search_manager.setOnSearchFinished(
        [&search_manager, &best_move]() { best_move = search_manager.bestMoveUci(); });
    search_manager.setPos(tc.fen);

    bitcrusher::SearchParameters params;
    params.move_time_ms = 15000;

    search_manager.startSearch<bitcrusher::FastMoveSink>(params);
    search_manager.waitUntilSearchFinished();

    if (! tc.best_moves.empty()) {
        bool found = false;
        for (const auto& bm : tc.best_moves) {
            if (best_move == bm) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Position: " << tc.fen << "\nExpected one of: " << [&] {
            std::string s;
            for (const auto& m : tc.best_moves) {
                s += m + " ";
            }
            return s;
        }() << "\nGot: " << best_move;
    }

    if (! tc.avoid_moves.empty()) {
        for (const auto& am : tc.avoid_moves) {
            EXPECT_NE(best_move, am)
                << "Position: " << tc.fen << "\nShould avoid: " << am << "\nGot: " << best_move;
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    EretTests,
    EretSearchTest,
    ::testing::ValuesIn(loadEretTestCases("data/epd/Eigenmann_Rapid_Engine_Test_uci.epd")),
    [](const testing::TestParamInfo<EretTestCase>& info) { return info.param.name; });
