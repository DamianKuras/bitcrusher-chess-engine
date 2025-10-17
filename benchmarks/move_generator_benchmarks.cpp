#include "benchmark/benchmark.h"
#include "benchmark_helper_functions.hpp"
#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "fen_formatter.hpp"
#include "legal_move_generators/legal_moves_generator.hpp"
#include "move_sink.hpp"
#include "restriction_context.hpp"
#include <cstddef>

static constexpr std::string_view INITIAL_POSITION_PATH = "../data/fens/initial_position.fen";
static constexpr std::string_view KIWIPETE_PATH         = "../data/fens/kiwipete.fen";
static constexpr std::string_view BRATKO_KOPEC_PATH     = "../data/epd/Bratko_Kopec.epd";
static constexpr int              REPETITION_COUNT      = 100;

using bench::utils::Epd;
using bench::utils::loadEPDsFromFile;
using bench::utils::loadFENFromFile;

class MoveGeneratorBenchmarksFixture : public benchmark::Fixture {
public:
    std::string                                 initial_position;
    std::string                                 kiwipete_position;
    bitcrusher::RestrictionContext              initial_position_restriction_context;
    bitcrusher::BoardState                      initial_position_board;
    bitcrusher::BoardState                      kiwipete_position_board;
    bitcrusher::RestrictionContext              kiwipete_position_restriction_context;
    std::vector<bench::utils::Epd>              bratko_kopec_epds;
    std::vector<bitcrusher::BoardState>         bratko_kopec_board_states;
    std::vector<bitcrusher::RestrictionContext> bratko_kopec_restriction_context;

    MoveGeneratorBenchmarksFixture() {
        initializePosition(loadFENFromFile(INITIAL_POSITION_PATH), initial_position_board,
                           initial_position_restriction_context);
        initializePosition(loadFENFromFile(KIWIPETE_PATH), kiwipete_position_board,
                           kiwipete_position_restriction_context);

        // Load and initialize Bratko-Kopec test suite.
        bratko_kopec_epds        = loadEPDsFromFile("../data/epd/Bratko_Kopec.epd");
        size_t bratko_kopec_size = bratko_kopec_epds.size();
        bratko_kopec_board_states.resize(bratko_kopec_size);
        bratko_kopec_restriction_context.resize(bratko_kopec_size);

        for (int i = 0; i < bratko_kopec_epds.size(); i++) {
            initializePosition(bratko_kopec_epds[i].fen, bratko_kopec_board_states[i],
                               bratko_kopec_restriction_context[i]);
        }
    }

private:
    static void initializePosition(const std::string&              fen,
                                   bitcrusher::BoardState&         board,
                                   bitcrusher::RestrictionContext& context) {
        bitcrusher::parseFEN(fen, board);
        if (board.isWhiteMove()) {
            bitcrusher::updateRestrictionContext<bitcrusher::Color::WHITE>(board, context);
        } else {
            bitcrusher::updateRestrictionContext<bitcrusher::Color::BLACK>(board, context);
        }
    }
};

BENCHMARK_DEFINE_F(MoveGeneratorBenchmarksFixture, GenerateMoves)(benchmark::State& state) {
    bitcrusher::FastMoveSink sink;
    uint64_t                 total_moves = 0;

    for (auto _ : state) {
        bitcrusher::generateLegalMoves<bitcrusher::Color::WHITE,
                                       bitcrusher::MoveGenerationPolicy::FULL,
                                       bitcrusher::RestrictionContextPolicy::LEAVE>(
            initial_position_board, sink, initial_position_restriction_context);
        total_moves += sink.count[0];
        bitcrusher::generateLegalMoves<bitcrusher::Color::WHITE,
                                       bitcrusher::MoveGenerationPolicy::FULL,
                                       bitcrusher::RestrictionContextPolicy::LEAVE>(
            kiwipete_position_board, sink, kiwipete_position_restriction_context);
        total_moves += sink.count[0];
        for (int i = 0; i < bratko_kopec_board_states.size(); i++) {
            if (bratko_kopec_board_states[i].isWhiteMove()) {
                bitcrusher::generateLegalMoves<bitcrusher::Color::WHITE>(
                    bratko_kopec_board_states[i], sink, bratko_kopec_restriction_context[i]);
                total_moves += sink.count[0];
            } else {
                bitcrusher::generateLegalMoves<bitcrusher::Color::BLACK>(
                    bratko_kopec_board_states[i], sink, bratko_kopec_restriction_context[i]);
                total_moves += sink.count[0];
            }
        }
        benchmark::DoNotOptimize(sink);

        state.counters["moves_per_second"] =
            benchmark::Counter(static_cast<double>(total_moves), benchmark::Counter::kIsRate);
    }
}

BENCHMARK_REGISTER_F(MoveGeneratorBenchmarksFixture, GenerateMoves)->Repetitions(REPETITION_COUNT);
