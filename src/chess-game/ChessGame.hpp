#pragma once

#include <array>
#include <optional>
#include <vector>

enum class PieceType { None, Pawn, Knight, Bishop, Rook, Queen, King };
enum class PieceColor { White, Black };
enum class GameStatus { Playing, Check, Checkmate, Stalemate, Draw };

struct Piece {
    PieceType type = PieceType::None;
    PieceColor color = PieceColor::White;
};

struct Square {
    int row = -1;
    int column = -1;
    bool operator==(const Square&) const = default;
};

struct ChessMove {
    Square from;
    Square to;
    PieceType promotion = PieceType::None;
    bool castle = false;
    bool enPassant = false;
};

class ChessGame {
public:
    ChessGame();
    void reset();
    const Piece& pieceAt(Square square) const;
    PieceColor turn() const { return turn_; }
    GameStatus status() const { return status_; }
    const std::optional<ChessMove>& lastMove() const { return lastMove_; }
    std::vector<ChessMove> legalMoves() const;
    std::vector<ChessMove> legalMovesFrom(Square square) const;
    bool move(const ChessMove& move);

private:
    using Board = std::array<std::array<Piece, 8>, 8>;
    static bool valid(Square square);
    static PieceColor opposite(PieceColor color);
    bool attacked(const Board& board, Square square, PieceColor by) const;
    bool inCheck(const Board& board, PieceColor color) const;
    std::vector<ChessMove> pseudoMoves(const Board& board, PieceColor color) const;
    void appendPieceMoves(const Board& board, Square from,
                          std::vector<ChessMove>& moves) const;
    Board applied(const Board& board, const ChessMove& move) const;
    void updateStatus();

    Board board_{};
    PieceColor turn_ = PieceColor::White;
    bool whiteKingCastle_ = true;
    bool whiteQueenCastle_ = true;
    bool blackKingCastle_ = true;
    bool blackQueenCastle_ = true;
    std::optional<Square> enPassantTarget_;
    std::optional<ChessMove> lastMove_;
    int halfmoveClock_ = 0;
    GameStatus status_ = GameStatus::Playing;
};
