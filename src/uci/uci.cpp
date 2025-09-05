#include "uci_handler.hpp"

static void initializeIO() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
}

int main(int argc, char* argv[]) {
    if (argc >= 2 && std::string(argv[1]) == "bench") {
        bitcrusher::uci::UCIHandler::handleBench();
        return 0;
    }

    initializeIO();
    bitcrusher::uci::UCIHandler uci;
    uci.run();
    return 0;
}