#include "uci_handler.hpp"

static void initializeIO() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
}

int main(int argc, char* argv[]) {
    std::span<char*> args{argv, static_cast<size_t>(argc)};
    if (args.size() >= 2 && std::string(args[1]) == "bench") {
        bitcrusher::uci::UCIHandler::handleBench();
        return 0;
    }

    initializeIO();
    bitcrusher::uci::UCIHandler uci;
    uci.run();
    return 0;
}