#include "uci_handler.hpp"

#include <cstddef>
#include <exception>
#include <ios>
#include <iostream>
#include <span>
#include <string>
#ifdef _WIN32
#    include <windows.h> // NOLINT(misc-include-cleaner)
#endif

static void initializeIO() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // NOLINT(misc-include-cleaner)
#endif
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
}

int main(int argc, char* argv[]) noexcept { // NOLINT(bugprone-exception-escape)
    try {
        const std::span<char*> args{argv, static_cast<std::size_t>(argc)};
        if (args.size() >= 2 && std::string(args[1]) == "bench") {
            bitcrusher::uci::UCIHandler::handleBench();
            return 0;
        }

        initializeIO();
        bitcrusher::uci::UCIHandler uci;
        uci.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Fatal error: unknown exception\n";
        return 1;
    }
}