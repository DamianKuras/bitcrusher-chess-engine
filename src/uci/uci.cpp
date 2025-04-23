#include "uci_handler.hpp"

static void initializeIO() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
}

int main() {
    initializeIO();
    bitcrusher::uci::UCIHandler uci;
    uci.run();
    return 0;
}