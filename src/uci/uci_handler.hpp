#ifndef BITCRUSHER_UCI_HANDLER_HPP
#define BITCRUSHER_UCI_HANDLER_HPP

#include "engine_debug_logger.hpp"
#include "move_sink.hpp"
#include "search_manager.hpp"
#include "uci_constants.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>

namespace bitcrusher::uci {

inline constexpr int BENCH_SEARCH_DEPTH = 5;
inline constexpr int UCI_LINE_MAX_LEN   = 300;

class UCIHandler {

public:
    UCIHandler() {
        line_.reserve(UCI_LINE_MAX_LEN);

        search_manager_.setOnSearchFinished([this]() {
            const std::string best_move = search_manager_.bestMoveUci();
            send(std::format("bestmove {}", best_move));
        });

        search_manager_.setOnDepthCompleted([this](int depth) {
            const uint64_t node_count     = search_manager_.getNodeCount();
            const uint64_t search_time_ms = getSearchTimeMs(search_manager_.getSearchStartTime(),
                                                            std::chrono::steady_clock::now());
            const uint64_t nps            = calculateNPS(node_count, search_time_ms);
            std::string    info =
                std::format("info depth {} score {} nodes {} time {} nps {} pv {}", depth,
                            search_manager_.getScore(), node_count, search_time_ms, nps,
                            search_manager_.getPrincipalVariation(depth));
#ifdef DEBUG
            std::string debug_info = std::format(
                " beta cutoffs {} tt cutoffs {} tt usage {}%", search_manager_.getBetaCutoffs(),
                search_manager_.getTTCutoffs(), search_manager_.getTTUsage());
            info += debug_info;
#endif
            send(info);
        });
    }

    static inline uint64_t calculateNPS(uint64_t nodes, uint64_t time_ms) noexcept {
        time_ms = std::max<uint64_t>(time_ms, 1); // Prevents division by zero.
        return (nodes * MILLISECONDS_PER_SECONDS) / time_ms;
    }

    static inline uint64_t
    getSearchTimeMs(std::chrono::time_point<std::chrono::steady_clock> start_time,
                    std::chrono::time_point<std::chrono::steady_clock> end_time) noexcept {
        uint64_t search_time_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        return search_time_ms;
    }

    void run() {
        while (std::getline(std::cin, line_)) {
#ifdef DEBUG
            logToFile("FROM GUI", line_);
#endif
            if (line_ == "quit") {
                break;
            }
            processCommand(line_);
        }
    }

    static inline void handleBench() {
        bitcrusher::SearchManager bench_search_manager{};
        bench_search_manager.setPosToStartpos();
        bitcrusher::SearchParameters params{.max_depth = BENCH_SEARCH_DEPTH};

        auto start_time = std::chrono::steady_clock::now();
        bench_search_manager.startSearch<FastMoveSink>(params);
        bench_search_manager.waitUntilSearchFinished();
        auto end_time = std::chrono::steady_clock::now();

        uint64_t nodes_searched       = bench_search_manager.getNodeCount();
        uint64_t duration_miliseconds = getSearchTimeMs(start_time, end_time);
        uint64_t nps                  = calculateNPS(nodes_searched, duration_miliseconds);
        send(std::format("{} nodes {} nps", nodes_searched, nps));
    }

private:
    std::string   line_;
    SearchManager search_manager_;

    static inline void send(std::string_view msg) {

#ifdef DEBUG
        logToFile("TO GUI", msg);
#endif
        std::cout << msg << "\n" << std::flush;
    }

    constexpr void processCommand(std::string_view input) {
        auto words =
            input | std::ranges::views::split(' ') |
            std::ranges::views::filter([](auto&& range) { return ! range.empty(); }) |
            std::ranges::views::transform([](auto&& range) { return std::string_view{range}; });
        auto words_iter     = words.begin();
        auto words_end_iter = words.end();

        if (words_iter == words.end()) {
            return;
        }
        std::string_view command_token = *words_iter;
        ++words_iter;

        if (command_token == "uci") {
            handleUCI();
        } else if (command_token == "isready") {
            handleIsready();
        } else if (command_token == "position") {
            handlePosition(words_iter, words_end_iter);
        } else if (command_token == "go") {
            handleGo(words_iter, words_end_iter);
        } else if (command_token == "stop") {
            handleStop();
        } else if (command_token == "setoption") {
            ++words_iter; // Skips over "name" token.
            std::string_view option_name = *words_iter;
            ++words_iter; // Skip over name value.
            ++words_iter; // Skips over "value" token.
            std::string_view value = *words_iter;
            handleSetOption(option_name, value);
        } else if (command_token == "ucinewgame") {
            search_manager_.newGame();
        } else if (command_token == "debug") {
            ++words_iter; // Skips over "name" token.
            handleDebug(*words_iter);
        } else if (command_token == "ponderhit") {
            handlePonderHit();
        } else if (command_token == "bench") {
            handleBench();
        } else if (command_token == "quit") {
            exit(0);
        } else {
            send("unknown command");
        }
    }

    static inline void handleIsready() { send("readyok"); }

    inline void handleStop() { search_manager_.stopSearch(); }

    constexpr void handleSetOption(std::string_view name, std::string_view value) {
        if (name == "Hash" || name == "hash") {
            int hash_size = HASH.default_value;
            parseNumber(value, hash_size);
            search_manager_.setHashMBSize(hash_size);
        }
        if (name == "threads" || name == "Threads") {
            int cores_count = THREADS.default_value;
            parseNumber(value, cores_count);
            search_manager_.setMaxCores(cores_count);
            send(std::format("info string Using {} threads", cores_count));
        }
    }

    static void handleUCI() { send(std::format("{}\n{}\nuciok", UCI_ID_STRING, OPTIONS)); }

    constexpr void handlePosition(auto iter, auto end_iter) {
        // Parse position description startpos or fen.
        search_manager_.resetMoveProcessor();
        if (*iter == "startpos") {
            search_manager_.setPosToStartpos();
            ++iter;
        } else if (*iter == "fen") {
            if (iter == end_iter) {
                send("unknown command");
                return;
            }
            ++iter; // Skips over fen token.
            std::string fen;
            int         i = 0;
            while (iter != end_iter && *iter != "moves") {
                if (i > 0) {
                    fen.append(" ");
                }
                fen.append(*iter);
                ++iter;
                ++i;
            }
            search_manager_.setPos(fen);
        }
        // Parse and apply moves.
        if (iter != end_iter && *iter == "moves") {
            ++iter;

            while (iter != end_iter) {
                search_manager_.applyUciMove(*iter);
                ++iter;
            }
        }
    }

    static inline void parseNumber(std::string_view number_text, int& number) {
        std::from_chars(number_text.data(), number_text.data() + number_text.size(), number);
    }

    constexpr void handleGo(auto iter, auto end_iter) {
        // Parse search parameters options.
        bitcrusher::SearchParameters params{};
        if (iter == end_iter) {
            params.infinite = true;
        }
        while (iter != end_iter) {
            std::string_view option = *iter;

            if (option == "ponder") {
                params.ponder = true;
            } else if (option == "wtime") {
                parseNumber(*++iter, params.white_time_ms);
            } else if (option == "btime") {
                parseNumber(*++iter, params.black_time_ms);
            } else if (option == "winc") {
                parseNumber(*++iter, params.white_increment_ms);
            } else if (option == "binc") {
                parseNumber(*++iter, params.black_increment_ms);
            } else if (option == "movestogo") {
                parseNumber(*++iter, params.moves_to_go);
            } else if (option == "depth") {
                parseNumber(*++iter, params.max_depth);
            } else if (option == "nodes") {
                parseNumber(*++iter, params.max_nodes);
            } else if (option == "mate") {
                parseNumber(*++iter, params.mate_in_x);
            } else if (option == "movetime") {
                parseNumber(*++iter, params.move_time_ms);
            } else if (option == "infinite") {
                params.infinite = true;
            } else if (option == "searchmoves") { // Searchmoves should be last.
                // Collect subsequent search moves until end.
                while (++iter != end_iter) {
                    params.addSearchMove(*iter);
                }
            } else if (option == "perft") {
                int perft_depth{0};
                parseNumber(*++iter, perft_depth);
                uint64_t nodes_count = search_manager_.performPerft(perft_depth);
                send(std::format("Nodes searched: {}", nodes_count));
                return;
            }
            ++iter;
        }
        search_manager_.startSearch<FastMoveSink>(params);
    }

    constexpr void handleDebug(std::string_view value) {
        if (value == "on") {
            search_manager_.setDebug(true);
        } else if (value == "off") {
            search_manager_.setDebug(false);
        }
    }

    constexpr void handlePonderHit() {}
};

} // namespace bitcrusher::uci

#endif // BITCRUSHER_UCI_HANDLER_HPP