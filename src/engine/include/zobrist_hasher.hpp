
#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "random"

namespace bitcrusher {

class ZobristHasher {
    inline static std::array<std::array<uint64_t, PIECE_COUNT>, SQUARE_COUNT> zobrist_piece_table;
    inline static uint64_t                                                    is_black_move_zobrist;
    inline static std::array<uint64_t, CASTLING_RIGHTS_COUNT> zobrist_castling_rights;
    inline static std::array<uint64_t, BOARD_DIMENSION>       zobrist_en_passant_file;

public:
    static void init(uint64_t seed) {
        std::mt19937_64 rng(seed);
        for (int i = 0; i < SQUARE_COUNT; i++) {
            for (int j = 0; j < PIECE_COUNT; j++) {
                zobrist_piece_table[i][j] = rng();
            }
        }
        is_black_move_zobrist = rng();
        for (auto& key : zobrist_castling_rights) {
            key = rng();
        }
        for (auto& key : zobrist_en_passant_file) {
            key = rng();
        }
    }

    // Compute 64-bit Zobrist hash for a given board position
    [[nodiscard]] static uint64_t createHash(const BoardState& board) {
        uint64_t hash = 0;
        for (int sq = 0; sq < SQUARE_COUNT; sq++) {
            Piece piece = board.getPieceOnSquare(Square(sq));
            if (piece != Piece::NONE) {
                assert(static_cast<int>(piece) < PIECE_COUNT &&
                       "ZobristHasher: PIECE_COUNT mismatch");

                hash ^= zobrist_piece_table[sq][static_cast<int>(piece)];
            }
        }
        // side to move
        if (! board.isWhiteMove()) {
            hash ^= is_black_move_zobrist;
        }
        // castling rights
        if (board.hasWhiteKingsideCastlingRight()) {
            hash ^= zobrist_castling_rights[0];
        }
        if (board.hasWhiteQueensideCastlingRight()) {
            hash ^= zobrist_castling_rights[1];
        }
        if (board.hasBlackKingsideCastlingRight()) {
            hash ^= zobrist_castling_rights[2];
        }
        if (board.hasBlackQueensideCastlingRight()) {
            hash ^= zobrist_castling_rights[3];
        }
        // en passant file
        if (board.hasEnPassant()) {
            int en_passant_file = static_cast<int>(convert::toFile(board.getEnPassantSquare()));
            assert(en_passant_file >= 0 && en_passant_file < BOARD_DIMENSION &&
                   "ZobristHasher: BOARD_DIMENSION mismatch");
            hash ^= zobrist_en_passant_file[en_passant_file];
        }
        return hash;
    }
};
} // namespace bitcrusher
