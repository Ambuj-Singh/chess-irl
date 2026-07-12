#include "chess-game/ChessGame.hpp"

#include <cassert>

namespace {
bool play(ChessGame& game, Square from, Square to) {
    return game.move({from, to});
}
}

int main() {
    ChessGame game;

    assert(game.turn() == PieceColor::White);
    assert(!play(game, {1, 4}, {3, 4})); // Black cannot move first.
    assert(!play(game, {7, 0}, {5, 0})); // The rook is blocked.
    assert(play(game, {6, 4}, {4, 4}));  // e2-e4.
    assert(game.turn() == PieceColor::Black);
    assert(!play(game, {4, 4}, {3, 4})); // White cannot move twice.

    // Fool's mate: 1. f3 e5 2. g4 Qh4#.
    game.reset();
    assert(play(game, {6, 5}, {5, 5}));
    assert(play(game, {1, 4}, {3, 4}));
    assert(play(game, {6, 6}, {4, 6}));
    assert(play(game, {0, 3}, {4, 7}));
    assert(game.status() == GameStatus::Checkmate);
    assert(game.legalMoves().empty());

    return 0;
}
