#ifndef BITCRUSHER_PAWN_LEGAL_MOVES_HPP
#define BITCRUSHER_PAWN_LEGAL_MOVES_HPP

#include "attack_generators/pawn_attacks.hpp"
#include "bitboard_concepts.hpp"
#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"
#include "checks_pins_detection.hpp"
#include "move_generation_from_bitboard.hpp"
#include "restriction_context.hpp"

namespace bitcrusher {

template <Color Side, Direction PawnAttackDirection>
    requires Horizontal<PawnAttackDirection>
constexpr bool isValidEnPassant(const BoardState&         board,
                                const RestrictionContext& restriction_context,
                                uint64_t                  en_passant_bitboard) {

    constexpr uint64_t EN_PASSANT_RANK_BITBOARD = getEnPassantRankMask<Side>();

    uint64_t pawn_capturing_on_ep_square =
        offset::shiftBitboardNoWrap<offset::getOppositeDirection<PawnAttackDirection>(),
                                    offset::getOppositeDirection<getPawnPushDirection<Side>()>()>(
            en_passant_bitboard) &
        board.getBitboard<PieceType::PAWN, Side>();

    // No valid pawn attack on en passant square
    if (pawn_capturing_on_ep_square == EMPTY_BITBOARD) {
        return false;
    }
    // pawns is pinned horizontally or vertically
    if ((pawn_capturing_on_ep_square & restriction_context.pinmask_horizontal_vertical) !=
        EMPTY_BITBOARD) {
        return false;
    }

    if ((pawn_capturing_on_ep_square & restriction_context.pinmask_diagonal) != EMPTY_BITBOARD) {
        if ((en_passant_bitboard & restriction_context.pinmask_diagonal) == EMPTY_BITBOARD) {
            return false;
        }
    }
    // We have a valid pawn attacks on the ep square and our king is not on
    // ep_rank
    if ((board.getBitboard<PieceType::KING, Side>() & EN_PASSANT_RANK_BITBOARD) == EMPTY_BITBOARD) {
        return true;
    }
    // Our king is on en passant rank as well as capturing and captured pawns
    // en_passant could expose a check to the king
    uint64_t pawn_captured_by_ep =
        offset::shiftBitboardNoWrap<PawnAttackDirection>(pawn_capturing_on_ep_square);
    // Temporary remove both capturing and captured pawn from occupancy
    uint64_t temporary_occupancy =
        board.getAllOccupancy() ^ pawn_capturing_on_ep_square ^ pawn_captured_by_ep;

    return ! static_cast<bool>(isCheckedHorizontallyOnRank(
        board.getBitboard<PieceType::KING, Side>(), temporary_occupancy,
        board.getHorizontalVerticalSliders<convert::toOppositeColor<Side>()>(),
        EN_PASSANT_RANK_BITBOARD));
}

// Generates pawn attack moves for a specific attack direction
template <MoveSink MoveSinkT, Color Side, Direction PawnAttackDirection>
    requires Horizontal<PawnAttackDirection>
void generatePawnAttackMoves(const BoardState&         board,
                             uint64_t                  pawns_not_pinned,
                             uint64_t                  pawns_pinned_only_d,
                             const RestrictionContext& restriction_context,
                             MoveSinkT&                sink) {
    // const uint64_t pawns_left_attacks = generatePawnSingleSideAttacks<Side,
    // PawnAttackDirection>();

    const uint64_t pawns_valid_side_attacks =
        (((generatePawnSingleSideAttacks<Side, PawnAttackDirection>(pawns_not_pinned)) |
          ((generatePawnSingleSideAttacks<Side, PawnAttackDirection>(
              pawns_pinned_only_d)))&restriction_context.pinmask_diagonal)) &
        board.getOpponentOccupancy<Side>() & restriction_context.checkmask;

    uint64_t pawns_side_attacks_promotions = pawns_valid_side_attacks & PROMOTION_RANKS_MASK;
    uint64_t pawns_side_attacks_non_promotions =
        pawns_valid_side_attacks & NON_PROMOTION_RANKS_MASK;

    createMovesFromBitboard<MoveType::CAPTURE, PieceType::PAWN>(
        sink, pawns_side_attacks_non_promotions, getPawnAttackOffset<Side, PawnAttackDirection>(),
        board);

    createMovesFromBitboard<MoveType::PROMOTION_CAPTURE, PieceType::PAWN>(
        sink, pawns_side_attacks_promotions, getPawnAttackOffset<Side, PawnAttackDirection>(),
        board);

    if (board.hasEnPassant()) {
        uint64_t en_passant_bitboard = convert::toBitboard(board.getEnPassantSquare());

        if (isValidEnPassant<Side, PawnAttackDirection>(board, restriction_context,
                                                        en_passant_bitboard)) {
            createMovesFromBitboard<MoveType::EN_PASSANT, PieceType::PAWN>(
                sink, en_passant_bitboard, getPawnAttackOffset<Side, PawnAttackDirection>(), board);
        }
    }
}

template <MoveSink MoveSinkT, Color Side>
void generateLegalPawnMoves(const BoardState&         board,
                            const RestrictionContext& restriction_context,
                            MoveSinkT&                sink) {
    // const uint64_t pawns_not_pinned = board.getBitboard<PieceType::PAWN, Side>() &
    //                                   ~(board.restriction_context.pinmask_diagonal |
    //                                     restriction_context.pinmask_horizontal_vertical);
    const uint64_t pawns_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::PAWN, Side>());

    const uint64_t pawns_pinned_only_hv = board.getBitboard<PieceType::PAWN, Side>() &
                                          (~restriction_context.pinmask_diagonal) &
                                          restriction_context.pinmask_horizontal_vertical;

    const uint64_t pawns_pinned_only_d = board.getBitboard<PieceType::PAWN, Side>() &
                                         (~restriction_context.pinmask_horizontal_vertical) &
                                         restriction_context.pinmask_diagonal;

    // Attacks and en_passant
    generatePawnAttackMoves<MoveSinkT, Side, Direction::LEFT>(
        board, pawns_not_pinned, pawns_pinned_only_d, restriction_context, sink);
    generatePawnAttackMoves<MoveSinkT, Side, Direction::RIGHT>(
        board, pawns_not_pinned, pawns_pinned_only_d, restriction_context, sink);

    // Single pawn pushes
    uint64_t single_pawn_pushes = ((pawnSinglePush<Side>(pawns_not_pinned) |
                                    ((pawnSinglePush<Side>(pawns_pinned_only_hv)) &
                                     restriction_context.pinmask_horizontal_vertical))) &
                                  board.getEmptySquares() & restriction_context.checkmask;
    uint64_t pawn_pushes_non_promotions = single_pawn_pushes & NON_PROMOTION_RANKS_MASK;
    createMovesFromBitboard<MoveType::QUIET, PieceType::PAWN>(sink, pawn_pushes_non_promotions,
                                                              pawnPushOffset<Side>(), board);

    uint64_t pawn_pushes_promotions = single_pawn_pushes & PROMOTION_RANKS_MASK;
    createMovesFromBitboard<MoveType::PROMOTION, PieceType::PAWN>(sink, pawn_pushes_promotions,
                                                                  pawnPushOffset<Side>(), board);

    // Double pawn pushes
    // we can't push over occupied square
    const uint64_t double_push_able_pawns_not_pinned = getPawnsOnStartRank<Side>(pawns_not_pinned);
    const uint64_t temporary_single_push_not_pinned =
        pawnSinglePush<Side>(double_push_able_pawns_not_pinned) & board.getEmptySquares();
    uint64_t double_pawn_pushes_not_pinned =
        pawnSinglePush<Side>(temporary_single_push_not_pinned) & board.getEmptySquares() &
        restriction_context.checkmask;
    createMovesFromBitboard<MoveType::DOUBLE_PAWN_PUSH, PieceType::PAWN>(
        sink, double_pawn_pushes_not_pinned, pawnDoublePushOffset<Side>(), board);

    const uint64_t double_push_able_pawns_pinned_hv =
        getPawnsOnStartRank<Side>(pawns_pinned_only_hv);
    uint64_t temporary_single_push_pinned_hv =
        pawnSinglePush<Side>(double_push_able_pawns_pinned_hv) & board.getEmptySquares();
    uint64_t double_pawn_pushes_pinned_hv =
        pawnSinglePush<Side>(temporary_single_push_pinned_hv) & board.getEmptySquares() &
        restriction_context.checkmask & restriction_context.pinmask_horizontal_vertical;

    createMovesFromBitboard<MoveType::DOUBLE_PAWN_PUSH, PieceType::PAWN>(
        sink, double_pawn_pushes_pinned_hv, pawnDoublePushOffset<Side>(), board);
}

} // namespace bitcrusher

#endif // BITCRUSHER_PAWN_LEGAL_MOVES_HPP