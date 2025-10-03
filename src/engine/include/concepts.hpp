#ifndef BITCRUSHER_CONCEPTS_HPP
#define BITCRUSHER_CONCEPTS_HPP

#include "bitboard_enums.hpp"
#include "move.hpp"
#include <concepts>
#include <generator>

namespace bitcrusher {

template <typename Derived> struct MoveSinkBase {
    template <MoveType  MoveT,
              PieceType MovedOrPromotedToPiece,
              Color     SideToMove,
              PieceType CapturedPiece = PieceType::NONE>
    void emplace(Square from, Square to) noexcept {
        // Interface enforcement via CRTP
        static_cast<Derived*>(this)
            ->template emplace<MoveT, MovedOrPromotedToPiece, SideToMove, CapturedPiece>(from, to);
    }
};

template <typename T>
concept MoveSink = requires { requires std::derived_from<T, MoveSinkBase<T>>; };

template <Direction D>
concept Horizontal = (D == Direction::LEFT || D == Direction::RIGHT);

template <auto G>
concept DirectionalAttackGenerator = requires(uint64_t from, uint64_t occ) {
    { G(from, occ) } -> std::same_as<uint64_t>;
};

} // namespace bitcrusher

#endif // BITCRUSHER_CONCEPTS_HPP