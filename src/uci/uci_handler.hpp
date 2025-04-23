#ifndef BITCRUSHER_UCI_HANDLER_HPP
#define BITCRUSHER_UCI_HANDLER_HPP

#include "engine.hpp"
#include "engine_debug_logger.hpp"
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

    static inline void handleUCI() {
        std::string_view uci_string = "id name Bitcrusher Chess Engine 0.1 \n"
                                      "id author Damian Kuraś \n"
                                      "uciok";
        send(uci_string);
    }

    constexpr void handlePosition(auto iter, auto end_iter) {}

    constexpr void handleGo(auto iter, auto end_iter) {}

    constexpr void handlePonderHit() {}
};

} // namespace bitcrusher::uci

#endif // BITCRUSHER_UCI_HANDLER_HPP