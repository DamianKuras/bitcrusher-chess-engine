#include "perft_fixture.hpp"
#include <gtest/gtest.h>
using test_helpers::PerftParametrizedTest;
using test_helpers::PerftTestCase;

const std::string INITIAL_POSITION_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

const std::array<PerftTestCase, 10> PERFT_TEST_CASES{{
    // source of data for tests cases https://www.chessprogramming.org/Perft_Results
    {.name            = "perft_initial_position_depth_1",
     .fen             = INITIAL_POSITION_FEN,
     .depth           = 1,
     .leaf_node_count = 20},

    {.name            = "perft_inital_position_depth_2",
     .fen             = INITIAL_POSITION_FEN,
     .depth           = 2,
     .leaf_node_count = 400},

    {.name            = "perft_inital_position_depth_3",
     .fen             = INITIAL_POSITION_FEN,
     .depth           = 3,
     .leaf_node_count = 8902,
     .captures_count  = 34},

    {.name             = "perft_inital_position_depth_6",
     .fen              = INITIAL_POSITION_FEN,
     .depth            = 6,
     .leaf_node_count  = 119060324,
     .captures_count   = 2812008,
     .en_passant_count = 5248},

    {.name             = "perft_inital_position_depth_7_slow",
     .fen              = INITIAL_POSITION_FEN,
     .depth            = 7,
     .leaf_node_count  = 3195901860,
     .captures_count   = 108329926,
     .en_passant_count = 319617,
     .castling_count   = 883453},

    {.name             = "perft_inital_position_depth_8_slow",
     .fen              = INITIAL_POSITION_FEN,
     .depth            = 8,
     .leaf_node_count  = 84998978956,
     .captures_count   = 3523740106,
     .en_passant_count = 7187977,
     .castling_count   = 23605205},

    {.name             = "perft_kiwipete_position_depth_3",
     .fen              = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
     .depth            = 3,
     .leaf_node_count  = 97862,
     .captures_count   = 17102,
     .en_passant_count = 45,
     .castling_count   = 3162},

    {.name             = "perft_kiwipete_position_depth_6_slow",
     .fen              = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
     .depth            = 6,
     .leaf_node_count  = 8031647685,
     .captures_count   = 1558445089,
     .en_passant_count = 3577504,
     .promotions_count = 56627920,
     .castling_count   = 184513607},

    {.name             = "perft_mirroring_result_position_depth_3",
     .fen              = "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ",
     .depth            = 3,
     .leaf_node_count  = 9467,
     .captures_count   = 1021,
     .en_passant_count = 4,
     .promotions_count = 120},

    {.name             = "perft_mirroring_result_position_depth_6_slow",
     .fen              = "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ",
     .depth            = 6,
     .leaf_node_count  = 706045033,
     .captures_count   = 210369132,
     .en_passant_count = 212,
     .promotions_count = 81102984,
     .castling_count   = 10882006},

}};

INSTANTIATE_TEST_SUITE_P(PerftTests,
                         PerftParametrizedTest,
                         ::testing::ValuesIn(PERFT_TEST_CASES),
                         [](const testing::TestParamInfo<PerftTestCase>& info) {
                             return info.param.name;
                         });