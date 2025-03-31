

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "bitboard_enums.hpp"
#include "bitboard_utils.hpp"
#include "board_state.hpp"
#include <iostream>
#include <string>

namespace bitcrusher::debug {

constexpr std::array RANK_ORDER{Rank::R_8, Rank::R_7, Rank::R_6, Rank::R_5,
                                Rank::R_4, Rank::R_3, Rank::R_2, Rank::R_1};

constexpr std::array FILE_ORDER{File::A, File::B, File::C, File::D,
                                File::E, File::F, File::G, File::H};

inline void printBitboard(uint64_t bitboard, const std::string& message = "") {
    if (message != "") {
        std::cout << message << "\n";
    }
    std::cout << "\n";
    for (auto rank : RANK_ORDER) {
        std::cout << "r" << convert::toChar(rank) << " ";
        for (auto file : FILE_ORDER) {
            bitcrusher::Square sq = bitcrusher::convert::toSquare(file, rank);
            std::cout << utils::isSquareSet(bitboard, sq) << " ";
        }

        std::cout << '\n';
    }
    std::cout << "\n   a b c d e f g h" << "\n";
    std::cout << "Bitboard value: " << bitboard << "\n \n";
}

inline void printBoard(const BoardState& board, const std::string& message = "") {
    if (message != "") {
        std::cout << message << "\n";
    }
    std::cout << "\n";
    for (auto rank : RANK_ORDER) {
        std::cout << "r" << convert::toChar(rank) << " ";
        for (auto file : FILE_ORDER) {
            bitcrusher::Square sq = bitcrusher::convert::toSquare(file, rank);
            std::cout << convert::toChar(board.getPieceOnSquare(sq)) << " ";
        }

        std::cout << "\n";
    }
    std::cout << "\n   a b c d e f g h" << "\n";
}
} // namespace bitcrusher::debug

#endif