#ifndef BITCRUSHER_UCI_HANDLER_HPP
#define BITCRUSHER_UCI_HANDLER_HPP

#include "engine.hpp"
#include "engine_debug_logger.hpp"
#include "uci_constants.hpp"
#include <chrono>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <thread>

namespace bitcrusher::uci {

class UCIHandler {

public:
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

private:
    bool               new_game_{true};
    std::string        line_;
    bitcrusher::Engine engine_;
    std::jthread       search_thread_;
    std::stop_source   stop_source_;

    static inline void send(std::string_view msg) {

#ifdef DEBUG
        logToFile("TO GUI", msg);
#endif

        std::cout << msg << "\n" << std::flush;
    }

    [[nodiscard]] inline std::string getInfo() const {
        auto const&                   ctx = engine_.getSearchContext();
        std::chrono::duration<double> elapsed_seconds{std::chrono::steady_clock::now() -
                                                      ctx.start_time};
        uint64_t                      nps = ctx.nodes_searched / elapsed_seconds.count();

        auto elapsed_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_seconds).count();

        std::string info = std::format("info depth {} nodes {} time {} nps {}", ctx.depth,
                                       ctx.nodes_searched, elapsed_ms, nps);

        if (abs(ctx.best_eval) > CHECKMATE_THRESHOLD) {
            const int mate_plies = CHECKMATE_BASE - abs(ctx.best_eval);
            const int mate_in    = (mate_plies + 1) / 2;
            info += std::format(" score mate {}", ctx.best_eval > 0 ? mate_in : -mate_in);
        } else {
            info += std::format(" score cp {}", ctx.best_eval);
        }
        return info;
    }

    constexpr void processCommand(std::string_view input) {
        auto words = input | std::ranges::views::split(' ') |
                     std::ranges::views::filter([](auto&& range) { return ! range.empty(); });
        auto             words_iter     = words.begin();
        auto             words_end_iter = words.end();
        std::string_view command_token  = std::string_view{*words_iter};
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
            ++words_iter; // skips over "name" token
            std::string_view option_name{*words_iter};
            ++words_iter; // skips over "value" token
            std::string_view value{*words_iter};
            handleSetOption(option_name, value);
        } else if (command_token == "ucinewgame") {
            new_game_ = true;
        } else if (command_token == "ponderhit") {
            handlePonderHit();
        } else if (command_token == "quit") {
            exit(0);
        } else {
            send("unknown command");
        }
    }

    static inline void handleIsready() { send("readyok"); }

    inline void handleStop() { stop_source_.request_stop(); }

    constexpr void handleSetOption(std::string_view name, std::string_view value) {}

    static inline void handleUCI() { send(UCI_ID_STRING); }

    constexpr void handlePosition(auto iter, auto end_iter) {
        // parse further options startpos fen and moves
        if (std::string_view{*iter} == "startpos") {
            engine_.setPosToStartpos();
            ++iter;
        } else if (std::string_view{*iter} == "fen") {
            ++iter;
            std::string fen;
            int         i = 0;
            while (iter != end_iter && std::string_view{*iter} != "moves") {
                if (i > 0) {
                    fen.append(" ");
                }
                fen.append(std::string_view{*iter});
                ++iter;
                ++i;
            }
            engine_.setPos(fen);
            return;
        }
        if (iter != end_iter && std::string_view{*iter} == "moves") {
            // parse moves after position
            while (iter != end_iter) {
                engine_.applyUciMove(std::string_view{*iter});
                ++iter;
            }
        }
    }

    inline void parseNumber(auto range, int& number) {
        const std::string_view number_text = std::string_view{*range};
        std::from_chars(number_text.data(), number_text.data() + number_text.size(), number);
    }

    constexpr void handleGo(auto iter, auto end_iter) {
        // parse search parameters options
        bitcrusher::SearchParameters params{};
        while (iter != end_iter) {
            std::string_view option{*iter};
            ++iter;
            if (option == "ponder") {
                params.ponder = true;
            } else if (option == "wtime") {
                parseNumber(iter, params.white_time_ms);
            } else if (option == "btime") {
                parseNumber(iter, params.black_time_ms);
                ++iter;
            } else if (option == "winc") {
                parseNumber(iter, params.white_increment_ms);
                ++iter;
            } else if (option == "binc") {
                parseNumber(iter, params.black_increment_ms);
                ++iter;
            } else if (option == "movestogo") {
                parseNumber(iter, params.moves_to_go);
                ++iter;
            } else if (option == "depth") {
                parseNumber(iter, params.max_depth);
                ++iter;
            } else if (option == "nodes") {
                parseNumber(iter, params.max_nodes);
                ++iter;
            } else if (option == "mate") {
                parseNumber(iter, params.mate_in_x);
                ++iter;
            } else if (option == "movetime") {
                parseNumber(iter, params.move_time_ms);
                ++iter;
            } else if (option == "infinite") {
                params.infinite = true;
            } else if (option == "searchmoves") { // searchmoves should be last
                // Collect subsequent search moves until end.
                while (iter != end_iter) {
                    params.addSearchMove(std::string_view{*iter});
                    ++iter;
                }
            }
        }

        auto stop_token = stop_source_.get_token();
        search_thread_ =
            std::jthread([this, params, stop_token]() { engine_.go(params, stop_token); });
        search_thread_.join();
        send("bestmove " + engine_.getBestMoveUci() + "\n" + getInfo());
    }

    constexpr void handlePonderHit() {}
};

} // namespace bitcrusher::uci

#endif // BITCRUSHER_UCI_HANDLER_HPP