#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "fen_formatter.hpp"
#include "move_processor.hpp"
#include "zobrist_hasher.hpp"
#include <cstdint>
#include <gtest/gtest.h>

using bitcrusher::ZobristHasher;

class ZobristHasherTest : public ::testing::Test {
protected:
    void SetUp() override { ZobristHasher::init(12345); }

private:
};

TEST_F(ZobristHasherTest, ConsistentHashForSamePosition) {
    bitcrusher::BoardState board;
    bitcrusher::parseFEN(bitcrusher::INITIAL_POSITION_FEN, board);

    const uint64_t hash1 = ZobristHasher::createHash(board);
    const uint64_t hash2 = ZobristHasher::createHash(board);

    EXPECT_EQ(hash1, hash2);
}

TEST_F(ZobristHasherTest, PieceChangesAffectHash) {
    bitcrusher::BoardState board;
    bitcrusher::parseFEN(bitcrusher::INITIAL_POSITION_FEN, board);
    bitcrusher::MoveProcessor move_processor;

    const uint64_t hash1 = ZobristHasher::createHash(board);
    move_processor.applyMove(board, bitcrusher::moveFromUci("e2e4", board));
    const uint64_t hash2 = ZobristHasher::createHash(board);

    EXPECT_NE(hash1, hash2);
}

TEST_F(ZobristHasherTest, SideToMoveAffectsHash) {
    bitcrusher::BoardState board;
    bitcrusher::parseFEN(bitcrusher::INITIAL_POSITION_FEN, board);

    const uint64_t hash1 = ZobristHasher::createHash(board);
    board.setSideToMove(bitcrusher::Color::BLACK);
    const uint64_t hash2 = ZobristHasher::createHash(board);
    EXPECT_NE(hash1, hash2);
}

TEST_F(ZobristHasherTest, CastlingRightsAffectHash) {
    bitcrusher::BoardState board;
    bitcrusher::parseFEN(bitcrusher::INITIAL_POSITION_FEN, board);

    const uint64_t hash1 = ZobristHasher::createHash(board);
    board.removeBlackCastlingRights();
    const uint64_t hash2 = ZobristHasher::createHash(board);

    EXPECT_NE(hash1, hash2);
}

TEST_F(ZobristHasherTest, EnPassantAffectsHash) {
    bitcrusher::BoardState board;
    bitcrusher::parseFEN(bitcrusher::INITIAL_POSITION_FEN, board);

    const uint64_t hash1 = ZobristHasher::createHash(board);
    board.setEnPassantSquare(bitcrusher::Square::D4);
    const uint64_t hash2 = ZobristHasher::createHash(board);
    EXPECT_NE(hash1, hash2);
}
