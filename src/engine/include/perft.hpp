#ifndef BITCRUSHER_PERFT_HPP
#define BITCRUSHER_PERFT_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "concepts.hpp"
#include "legal_move_generators/legal_moves_generator.hpp"
#include "move_processor.hpp"
#include "restriction_context.hpp"

namespace bitcrusher {

template <Color SideToMove, MoveSink MoveSinkT>
[[nodiscard]] uint64_t perft(int                 depth,
                             BoardState&         board,
                             MoveProcessor&      move_processor,
                             MoveSinkT&          sink,
                             RestrictionContext& restriction_context,
                             int                 ply = 0) {
    uint64_t leaf_node_count = 0;

    generateLegalMoves<SideToMove>(board, sink, restriction_context, ply);
    if (depth == 1) {
        return static_cast<uint64_t>(sink.count[ply]);
    }
    for (std::size_t i = 0; i < sink.count[ply]; ++i) {
        move_processor.applyMove(board, sink.moves[ply][i]);
        leaf_node_count += perft<! SideToMove>(depth - 1, board, move_processor, sink,
                                               restriction_context, ply + 1);
        move_processor.undoMove(board, sink.moves[ply][i]);
    }
    return leaf_node_count;
}
} // namespace bitcrusher

#endif // BITCRUSHER_PERFT_HPP