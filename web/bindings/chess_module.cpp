#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h> // NOLINT(misc-include-cleaner)

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "evaluation.hpp"
#include "fen_formatter.hpp"
#include "legal_move_generators/legal_moves_generator.hpp"
#include "move.hpp"
#include "move_sink.hpp"
#include "pybind11/gil.h"
#include "restriction_context.hpp"
#include "search.hpp"
#include "search_manager.hpp"

namespace py = pybind11;

// Prefix all wrapper functions with bc to avoid name collisions with
// bitcrusher::search, bitcrusher::eval, etc. brought in via the engine headers.

constexpr int DEFAULT_TIME_LIMIT_MS = 10'000;
constexpr int MAX_SEARCH_DEPTH      = 100;
constexpr int MATE_PREFIX_LEN       = 5; // len("mate ")

// ---------------------------------------------------------------------------
// legal_moves(fen) -> list[str]
// ---------------------------------------------------------------------------

static std::vector<std::string> bcLegalMoves(const std::string& fen) {
    using bitcrusher::BoardState;
    using bitcrusher::Color;
    using bitcrusher::FastMoveSink;
    using bitcrusher::generateLegalMoves;
    using bitcrusher::parseFEN;
    using bitcrusher::RestrictionContext;
    using bitcrusher::toUci;

    BoardState         board;
    RestrictionContext ctx;
    FastMoveSink       sink;

    {
        const py::gil_scoped_release release;
        parseFEN(fen, board);
        if (board.isWhiteMove()) {
            generateLegalMoves<Color::WHITE>(board, sink, ctx, 0);
        } else {
            generateLegalMoves<Color::BLACK>(board, sink, ctx, 0);
        }
    }

    std::vector<std::string> moves;
    moves.reserve(sink.count[0]);
    for (int i = 0; i < sink.count[0]; ++i) {
        moves.push_back(toUci(sink.moves[0][i]));
    }
    return moves;
}

// ---------------------------------------------------------------------------
// evaluate(fen) -> int   (centipawns, relative to side to move)
// ---------------------------------------------------------------------------

static int bcEvaluate(const std::string& fen) {
    using bitcrusher::BoardState;
    using bitcrusher::eval;
    using bitcrusher::parseFEN;

    BoardState board;
    int        score = 0;
    {
        const py::gil_scoped_release release;
        parseFEN(fen, board);
        score = eval(board, board.getSideToMove());
    }
    return score;
}

// ---------------------------------------------------------------------------
// search(fen, depth, time_limit_ms) -> dict
//   Keys: score_cp (int|None), score_mate (int|None), best_move (str),
//         pv (list[str]), nodes (int)
// ---------------------------------------------------------------------------

static py::dict
bcSearch(const std::string& fen, int depth, int time_limit_ms = DEFAULT_TIME_LIMIT_MS) {
    using bitcrusher::FastMoveSink;
    using bitcrusher::SearchManager;
    using bitcrusher::SearchParameters;

    if (depth < 1 || depth > MAX_SEARCH_DEPTH) {
        throw std::invalid_argument("depth must be between 1 and 100");
    }
    if (time_limit_ms < 0) {
        throw std::invalid_argument("time_limit_ms must be non-negative");
    }

    SearchManager manager;
    manager.setPos(fen);

    SearchParameters params;
    // UCI "go depth N" maps to max_ply = N * 2 (iterative deepening steps).
    params.max_ply = depth * 2;
    // Prevent unbounded search when no time controls are set - calculateMoveTimeAllocation
    // returns INT_MAX when all time fields are zero, which effectively never terminates.
    params.move_time_ms = time_limit_ms > 0 ? time_limit_ms : DEFAULT_TIME_LIMIT_MS;

    {
        const py::gil_scoped_release release;
        manager.startSearch<FastMoveSink>(params);
        manager.waitUntilSearchFinished();
    }

    const std::string best_move = manager.bestMoveUci();
    const std::string score_str = manager.getScore(); // "cp X" or "mate X"
    const uint64_t    nodes     = manager.getNodeCount();
    const std::string pv_str    = manager.getPrincipalVariation(depth);

    // Split PV string into individual move tokens.
    std::vector<std::string> pv;
    std::istringstream       ss(pv_str);
    std::string              token{};
    while (ss >> token) {
        pv.push_back(token);
    }

    py::dict result;
    result["best_move"] = best_move;
    result["pv"]        = pv;
    result["nodes"]     = nodes;

    if (score_str.starts_with("cp ")) {
        result["score_cp"]   = std::stoi(score_str.substr(3));
        result["score_mate"] = py::none();
    } else if (score_str.starts_with("mate ")) {
        result["score_cp"]   = py::none();
        result["score_mate"] = std::stoi(score_str.substr(MATE_PREFIX_LEN));
    } else {
        result["score_cp"]   = py::none();
        result["score_mate"] = py::none();
    }

    return result;
}

// ---------------------------------------------------------------------------
// Module definition - must be at global scope (pybind11 macro expands to
// an extern "C" function, so it cannot live inside an anonymous namespace).
// ---------------------------------------------------------------------------
// NOLINTBEGIN
PYBIND11_MODULE(chess_engine, m) {
    m.doc() = "Bitcrusher chess engine Python bindings (pybind11)";

    m.def("legal_moves", &bcLegalMoves, py::arg("fen"),
          "Return all legal moves in UCI notation (e.g. ['e2e4', ...]) for the given FEN.");

    m.def("evaluate", &bcEvaluate, py::arg("fen"),
          "Static evaluation in centipawns, relative to the side to move. "
          "Positive = side to move is winning.");

    m.def("search", &bcSearch, py::arg("fen"), py::arg("depth") = 12,
          py::arg("time_limit_ms") = DEFAULT_TIME_LIMIT_MS,
          "Run iterative-deepening alpha-beta search. "
          "Returns a dict: {score_cp, score_mate, best_move, pv, nodes}.");
}

// NOLINTEND
