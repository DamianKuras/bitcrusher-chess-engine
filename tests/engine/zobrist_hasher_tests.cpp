#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "fen_formatter.hpp"
#include "move.hpp"
#include "move_processor.hpp"
#include "zobrist_hash_keys.hpp"
#include "zobrist_hasher.hpp"
#include <cstdint>
#include <gtest/gtest.h>

using bitcrusher::BoardState;
using bitcrusher::CastlingRights;
using bitcrusher::Color;
using bitcrusher::INITIAL_POSITION_FEN;
using bitcrusher::Move;
using bitcrusher::moveFromUci;
using bitcrusher::MoveProcessor;
using bitcrusher::parseFEN;
using bitcrusher::Square;
using bitcrusher::ZobristHasher;
using bitcrusher::ZobristKeys;

class ZobristHasherTest : public ::testing::Test {
protected:
    void SetUp() override {
        const int seed = 12345;
        ZobristKeys::init(seed);
    }

private:
};

TEST_F(ZobristHasherTest, ConsistentHashForSamePosition) {
    BoardState board;
    parseFEN(INITIAL_POSITION_FEN, board);

    const uint64_t hash1 = ZobristHasher::createHash(board);
    const uint64_t hash2 = ZobristHasher::createHash(board);

    EXPECT_EQ(hash1, hash2);
}

TEST_F(ZobristHasherTest, PieceChangesAffectHash) {
    BoardState board;
    parseFEN(INITIAL_POSITION_FEN, board);
    MoveProcessor move_processor;

    const uint64_t hash1 = ZobristHasher::createHash(board);
    move_processor.applyMove(board, moveFromUci("e2e4", board));
    const uint64_t hash2 = ZobristHasher::createHash(board);

    EXPECT_NE(hash1, hash2);
}

TEST_F(ZobristHasherTest, SideToMoveAffectsHash) {
    BoardState board;
    parseFEN(INITIAL_POSITION_FEN, board);

    const uint64_t hash1 = ZobristHasher::createHash(board);
    board.setSideToMove(Color::BLACK);
    const uint64_t hash2 = ZobristHasher::createHash(board);
    EXPECT_NE(hash1, hash2);
}

TEST_F(ZobristHasherTest, CastlingRightsAffectHash) {
    BoardState board;
    parseFEN(INITIAL_POSITION_FEN, board);

    const uint64_t hash1 = ZobristHasher::createHash(board);
    board.removeCastlingRights<CastlingRights::BLACK_CASTLING_RIGHTS>();
    // board.removeBlackCastlingRights();
    const uint64_t hash2 = ZobristHasher::createHash(board);

    EXPECT_NE(hash1, hash2);
}

TEST_F(ZobristHasherTest, EnPassantAffectsHash) {
    BoardState board;
    parseFEN(INITIAL_POSITION_FEN, board);

    const uint64_t hash1 = ZobristHasher::createHash(board);
    board.setEnPassantSquare(Square::D4);
    const uint64_t hash2 = ZobristHasher::createHash(board);
    EXPECT_NE(hash1, hash2);
}

TEST_F(ZobristHasherTest, HashStabilityAfterUndo) {
    BoardState board;
    parseFEN(INITIAL_POSITION_FEN, board);
    MoveProcessor move_processor;
    Move          move =
        Move::createQuietMove(Square::E2, bitcrusher::Square::E4, bitcrusher::PieceType::PAWN);
    auto h0 = ZobristHasher::createHash(board);
    move_processor.applyMove(board, move);
    move_processor.undoMove(board, move);
    EXPECT_EQ(ZobristHasher::createHash(board), h0);
}

TEST_F(ZobristHasherTest, CompositeCastlingRightsHash) {
    BoardState board;
    parseFEN(INITIAL_POSITION_FEN, board);

    uint64_t h_all = ZobristHasher::createHash(board);

    board.removeCastlingRights<CastlingRights::WHITE_CASTLING_RIGHTS>();
    uint64_t h_after = ZobristHasher::createHash(board);

    EXPECT_NE(h_all, h_after);
}

TEST_F(ZobristHasherTest, EnPassantHashClear) {
    BoardState board;
    parseFEN(INITIAL_POSITION_FEN, board);
    uint64_t h1 = ZobristHasher::createHash(board);

    board.setEnPassantSquare(Square::D4);
    uint64_t h2 = ZobristHasher::createHash(board);
    EXPECT_NE(h1, h2);

    board.setEnPassantSquare(Square::NULL_SQUARE);
    uint64_t h3 = ZobristHasher::createHash(board);
    EXPECT_EQ(h1, h3);
}

TEST_F(ZobristHasherTest, SideToMoveTemplatePaths) {
    BoardState board;
    parseFEN(INITIAL_POSITION_FEN, board);
    uint64_t h_white = ZobristHasher::createHash(board);

    board.setSideToMove(Color::BLACK);
    uint64_t h_black = ZobristHasher::createHash(board);
    EXPECT_NE(h_white, h_black);

    board.setSideToMove(Color::WHITE);
    uint64_t h_again = ZobristHasher::createHash(board);
    EXPECT_EQ(h_white, h_again);
}