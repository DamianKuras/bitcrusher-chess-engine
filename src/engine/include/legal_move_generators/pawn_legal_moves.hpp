#ifndef BITCRUSHER_PAWN_LEGAL_MOVES_HPP
#define BITCRUSHER_PAWN_LEGAL_MOVES_HPP

#include "attack_generators/pawn_attacks.hpp"
#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"
#include "concepts.hpp"
#include "restriction_context.hpp"
#include "shared_move_generation.hpp"

namespace bitcrusher {

/// @brief Validates whether an en passant capture is legal without exposing the king to a
/// horizontal check.
/// @tparam Side The Color of the side to move(Color::WHITE or Color::BLACK).
/// @tparam PawnAttackDirection Direction of the attack (LEFT or RIGHT).
/// @param board The current board state of the position.
/// @param restriction_context Contains check and pin informations.
/// @param en_passant_bitboard Bitboard with the en passant target square set.
/// @return True if the en passant capture is legal false otherwise.
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

    // No valid pawn attack on en passant square.
    if (pawn_capturing_on_ep_square == EMPTY_BITBOARD) {
        return false;
    }
    // Pawn is pinned horizontally or vertically.
    if ((pawn_capturing_on_ep_square & restriction_context.pinmask_horizontal_vertical) !=
        EMPTY_BITBOARD) {
        return false;
    }

    if ((pawn_capturing_on_ep_square & restriction_context.pinmask_diagonal) != EMPTY_BITBOARD) {
        if ((en_passant_bitboard & restriction_context.pinmask_diagonal) == EMPTY_BITBOARD) {
            return false;
        }
    }
    // We have a valid pawn attacks on the ep square and our king is not on ep_rank
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

/// @brief Generates pawn capture moves (including en passant and promotions) in a specific
/// direction.

/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @tparam Side The color of the side to move.
/// @tparam PawnAttackDirection Direction of the attack (LEFT or RIGHT). Pawns only attacks in the
/// direction they move towards but on the side of the pawn.
/// @param board The current board state of the position.
/// @param pawns_not_pinned Bitboard of unpinned pawns.
/// @param pawns_pinned_only_d Bitboard of pawns pinned only diagonally.
/// @param restriction_context Contains check and pin informations.
/// @param sink The move sink object that will store the generated capture moves.
template <MoveSink MoveSinkT, Color Side, Direction PawnAttackDirection>
    requires Horizontal<PawnAttackDirection>
void generatePawnCaptureMoves(const BoardState&         board,
                              uint64_t                  pawns_not_pinned,
                              uint64_t                  pawns_pinned_only_d,
                              const RestrictionContext& restriction_context,
                              MoveSinkT&                sink) {
    const uint64_t pawns_not_pinned_attacks =
        generatePawnSingleSideAttacks<Side, PawnAttackDirection>(pawns_not_pinned);
    const uint64_t pawns_pinned_attacks =
        (generatePawnSingleSideAttacks<Side, PawnAttackDirection>(pawns_pinned_only_d)) &
        restriction_context.pinmask_diagonal;

    const uint64_t pawns_valid_side_attacks = (pawns_not_pinned_attacks | pawns_pinned_attacks) &
                                              board.getOpponentOccupancy<Side>() &
                                              restriction_context.checkmask;

    uint64_t pawns_side_attacks_promotions = pawns_valid_side_attacks & PROMOTION_RANKS_MASK;
    uint64_t pawns_side_attacks_non_promotions =
        pawns_valid_side_attacks & NON_PROMOTION_RANKS_MASK;

    generateOrderedCapturesMVV_LVA<Side, PieceType::PAWN>(
        pawns_side_attacks_non_promotions, sink, board,
        getPawnAttackOffset<Side, PawnAttackDirection>());

    generateOrderedPromotionCaptures<Side>(pawns_side_attacks_promotions, sink, board,
                                           getPawnAttackOffset<Side, PawnAttackDirection>());

    if (board.hasEnPassant()) {
        uint64_t en_passant_bitboard = convert::toBitboard(board.getEnPassantSquare());

        if (isValidEnPassant<Side, PawnAttackDirection>(board, restriction_context,
                                                        en_passant_bitboard)) {
            createMovesFromBitboard<MoveType::EN_PASSANT, PieceType::PAWN, Side, PieceType::PAWN>(
                sink, en_passant_bitboard, getPawnAttackOffset<Side, PawnAttackDirection>());
        }
    }
}

/// @brief Generates all legal pawn moves for the given side, respecting restriction constraints.
/// @tparam Side The Color of the side to move(Color::WHITE or Color::BLACK).
/// @tparam MoveGenerationP MoveGenerationP Move generation scope policy. See MoveGenerationPolicy
/// for available
/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @param board The current board state of the position.
/// @param restriction_context Contains check and pin informations.
/// @param sink  The move sink object that will store the generated capture moves.
template <Color                Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateLegalPawnMoves(const BoardState&         board,
                            const RestrictionContext& restriction_context,
                            MoveSinkT&                sink) {
    const uint64_t pawns_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::PAWN, Side>());

    const uint64_t pawns_pinned_only_hv = board.getBitboard<PieceType::PAWN, Side>() &
                                          (~restriction_context.pinmask_diagonal) &
                                          restriction_context.pinmask_horizontal_vertical;

    const uint64_t pawns_pinned_only_d = board.getBitboard<PieceType::PAWN, Side>() &
                                         (~restriction_context.pinmask_horizontal_vertical) &
                                         restriction_context.pinmask_diagonal;

    // Attacks and en_passant
    generatePawnCaptureMoves<MoveSinkT, Side, Direction::LEFT>(
        board, pawns_not_pinned, pawns_pinned_only_d, restriction_context, sink);
    generatePawnCaptureMoves<MoveSinkT, Side, Direction::RIGHT>(
        board, pawns_not_pinned, pawns_pinned_only_d, restriction_context, sink);

    if constexpr (MoveGenerationP == MoveGenerationPolicy::CAPTURES_ONLY) {
        return;
    }
    // Single pawn pushes
    uint64_t single_pawn_pushes = ((pawnSinglePush<Side>(pawns_not_pinned) |
                                    ((pawnSinglePush<Side>(pawns_pinned_only_hv)) &
                                     restriction_context.pinmask_horizontal_vertical))) &
                                  board.getEmptySquares() & restriction_context.checkmask;
    uint64_t pawn_pushes_non_promotions = single_pawn_pushes & NON_PROMOTION_RANKS_MASK;
    createMovesFromBitboard<MoveType::QUIET, PieceType::PAWN, Side>(
        sink, pawn_pushes_non_promotions, pawnPushOffset<Side>());

    uint64_t pawn_pushes_promotions = single_pawn_pushes & PROMOTION_RANKS_MASK;
    createMovesFromBitboard<MoveType::PROMOTION, PieceType::PAWN, Side>(
        sink, pawn_pushes_promotions, pawnPushOffset<Side>());

    // Double pawn pushes
    const uint64_t double_push_able_pawns_not_pinned = getPawnsOnStartRank<Side>(pawns_not_pinned);
    const uint64_t temporary_single_push_not_pinned =
        pawnSinglePush<Side>(double_push_able_pawns_not_pinned) & board.getEmptySquares();
    uint64_t double_pawn_pushes_not_pinned =
        pawnSinglePush<Side>(temporary_single_push_not_pinned) & board.getEmptySquares() &
        restriction_context.checkmask;
    createMovesFromBitboard<MoveType::DOUBLE_PAWN_PUSH, PieceType::PAWN, Side>(
        sink, double_pawn_pushes_not_pinned, pawnDoublePushOffset<Side>());

    const uint64_t double_push_able_pawns_pinned_hv =
        getPawnsOnStartRank<Side>(pawns_pinned_only_hv);
    uint64_t temporary_single_push_pinned_hv =
        pawnSinglePush<Side>(double_push_able_pawns_pinned_hv) & board.getEmptySquares();
    uint64_t double_pawn_pushes_pinned_hv =
        pawnSinglePush<Side>(temporary_single_push_pinned_hv) & board.getEmptySquares() &
        restriction_context.checkmask & restriction_context.pinmask_horizontal_vertical;

    createMovesFromBitboard<MoveType::DOUBLE_PAWN_PUSH, PieceType::PAWN, Side>(
        sink, double_pawn_pushes_pinned_hv, pawnDoublePushOffset<Side>());
}

} // namespace bitcrusher

#endif // BITCRUSHER_PAWN_LEGAL_MOVES_HPP