#ifndef BITCRUSHER_ZOBRIST_HASHER_HPP
#define BITCRUSHER_ZOBRIST_HASHER_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "zobrist_hash_keys.hpp"
#include <cassert>

namespace bitcrusher {

class ZobristHasher {
public:
    // Compute 64-bit Zobrist hash for a given board position.
    [[nodiscard]] static uint64_t createHash(const BoardState& board) {
        uint64_t hash = 0;
        // Pieces.
        for (int sq = 0; sq < SQUARE_COUNT; sq++) {
            Piece piece = board.getPieceOnSquare(Square(sq));
            if (piece == Piece::NONE) {
                continue;
            }
            assert(static_cast<int>(piece) < PIECE_COUNT && "ZobristHasher: piece is not valid");
            hash ^= ZobristKeys::getPieceSquareKey(piece, Square(sq));
        }
        // Side to move.
        if (! board.isWhiteMove()) {
            hash ^= ZobristKeys::getIsBlackMoveKey();
        }
        // Castling rights.
        if (board.hasCastlingRights<CastlingRights::WHITE_KINGSIDE>()) {
            hash ^= ZobristKeys::getCastlingRightsKey<CastlingRights::WHITE_KINGSIDE>();
        }
        if (board.hasCastlingRights<CastlingRights::WHITE_QUEENSIDE>()) {
            hash ^= ZobristKeys::getCastlingRightsKey<CastlingRights::WHITE_QUEENSIDE>();
        }
        if (board.hasCastlingRights<CastlingRights::BLACK_KINGSIDE>()) {
            hash ^= ZobristKeys::getCastlingRightsKey<CastlingRights::BLACK_KINGSIDE>();
        }
        if (board.hasCastlingRights<CastlingRights::BLACK_QUEENSIDE>()) {
            hash ^= ZobristKeys::getCastlingRightsKey<CastlingRights::BLACK_QUEENSIDE>();
        }
        // En passant file.
        hash ^= ZobristKeys::getEnPassantKey(board.getEnPassantSquare());

        return hash;
    }
};

} // namespace bitcrusher

#endif // BITCRUSHER_ZOBRIST_HASHER_HPP