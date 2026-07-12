#include "ChessGame.hpp"

#include <algorithm>
#include <cmath>

namespace {
constexpr Piece EMPTY{};
constexpr std::array<PieceType, 8> BACK_RANK{
    PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen,
    PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook
};
}

ChessGame::ChessGame() { reset(); }

void ChessGame::reset() {
    for (auto& row : board_) for (auto& piece : row) piece = EMPTY;
    for (int column = 0; column < 8; ++column) {
        board_[0][column] = {BACK_RANK[column], PieceColor::Black};
        board_[1][column] = {PieceType::Pawn, PieceColor::Black};
        board_[6][column] = {PieceType::Pawn, PieceColor::White};
        board_[7][column] = {BACK_RANK[column], PieceColor::White};
    }
    turn_ = PieceColor::White;
    whiteKingCastle_ = whiteQueenCastle_ = true;
    blackKingCastle_ = blackQueenCastle_ = true;
    enPassantTarget_.reset();
    lastMove_.reset();
    halfmoveClock_ = 0;
    status_ = GameStatus::Playing;
}

bool ChessGame::valid(Square s) {
    return s.row >= 0 && s.row < 8 && s.column >= 0 && s.column < 8;
}

PieceColor ChessGame::opposite(PieceColor color) {
    return color == PieceColor::White ? PieceColor::Black : PieceColor::White;
}

const Piece& ChessGame::pieceAt(Square square) const {
    return valid(square) ? board_[square.row][square.column] : EMPTY;
}

bool ChessGame::attacked(const Board& board, Square target, PieceColor by) const {
    const int pawnDirection = by == PieceColor::White ? -1 : 1;
    for (int dc : {-1, 1}) {
        Square from{target.row - pawnDirection, target.column - dc};
        if (valid(from) && board[from.row][from.column].type == PieceType::Pawn &&
            board[from.row][from.column].color == by) return true;
    }
    constexpr int KNIGHT[][2]{{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (auto& offset : KNIGHT) {
        Square from{target.row + offset[0], target.column + offset[1]};
        if (valid(from) && board[from.row][from.column].type == PieceType::Knight &&
            board[from.row][from.column].color == by) return true;
    }
    constexpr int DIRECTIONS[][2]{{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1}};
    for (int d = 0; d < 8; ++d) {
        for (int distance = 1;; ++distance) {
            Square from{target.row + DIRECTIONS[d][0] * distance,
                        target.column + DIRECTIONS[d][1] * distance};
            if (!valid(from)) break;
            const Piece piece = board[from.row][from.column];
            if (piece.type == PieceType::None) continue;
            if (piece.color == by) {
                const bool straight = d < 4;
                if ((distance == 1 && piece.type == PieceType::King) ||
                    piece.type == PieceType::Queen ||
                    (straight && piece.type == PieceType::Rook) ||
                    (!straight && piece.type == PieceType::Bishop)) return true;
            }
            break;
        }
    }
    return false;
}

bool ChessGame::inCheck(const Board& board, PieceColor color) const {
    for (int row = 0; row < 8; ++row) for (int column = 0; column < 8; ++column) {
        const Piece piece = board[row][column];
        if (piece.type == PieceType::King && piece.color == color)
            return attacked(board, {row, column}, opposite(color));
    }
    return true;
}

void ChessGame::appendPieceMoves(const Board& board, Square from,
                                 std::vector<ChessMove>& moves) const {
    const Piece piece = board[from.row][from.column];
    auto add = [&](Square to, bool castle = false, bool ep = false) {
        if (!valid(to)) return false;
        const Piece target = board[to.row][to.column];
        if (target.type != PieceType::None && target.color == piece.color) return false;
        moves.push_back({from, to, PieceType::None, castle, ep});
        return target.type == PieceType::None;
    };
    if (piece.type == PieceType::Pawn) {
        const int direction = piece.color == PieceColor::White ? -1 : 1;
        const int start = piece.color == PieceColor::White ? 6 : 1;
        Square one{from.row + direction, from.column};
        if (valid(one) && board[one.row][one.column].type == PieceType::None) {
            add(one);
            Square two{from.row + direction * 2, from.column};
            if (from.row == start && board[two.row][two.column].type == PieceType::None) add(two);
        }
        for (int dc : {-1, 1}) {
            Square to{from.row + direction, from.column + dc};
            if (!valid(to)) continue;
            if (board[to.row][to.column].type != PieceType::None &&
                board[to.row][to.column].color != piece.color) add(to);
            else if (enPassantTarget_ && to == *enPassantTarget_) add(to, false, true);
        }
        return;
    }
    if (piece.type == PieceType::Knight) {
        constexpr int OFFSETS[][2]{{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
        for (auto& o : OFFSETS) add({from.row + o[0], from.column + o[1]});
        return;
    }
    if (piece.type == PieceType::King) {
        for (int dr = -1; dr <= 1; ++dr) for (int dc = -1; dc <= 1; ++dc)
            if (dr || dc) add({from.row + dr, from.column + dc});
        const bool kingSide = piece.color == PieceColor::White ? whiteKingCastle_ : blackKingCastle_;
        const bool queenSide = piece.color == PieceColor::White ? whiteQueenCastle_ : blackQueenCastle_;
        const PieceColor enemy = opposite(piece.color);
        if (!attacked(board, from, enemy)) {
            if (kingSide && board[from.row][5].type == PieceType::None &&
                board[from.row][6].type == PieceType::None &&
                board[from.row][7].type == PieceType::Rook &&
                !attacked(board, {from.row,5}, enemy) && !attacked(board, {from.row,6}, enemy))
                add({from.row,6}, true);
            if (queenSide && board[from.row][1].type == PieceType::None &&
                board[from.row][2].type == PieceType::None && board[from.row][3].type == PieceType::None &&
                board[from.row][0].type == PieceType::Rook &&
                !attacked(board, {from.row,3}, enemy) && !attacked(board, {from.row,2}, enemy))
                add({from.row,2}, true);
        }
        return;
    }
    constexpr int DIRECTIONS[][2]{{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1}};
    int begin = piece.type == PieceType::Bishop ? 4 : 0;
    int end = piece.type == PieceType::Rook ? 4 : 8;
    for (int d = begin; d < end; ++d) for (int distance = 1; distance < 8; ++distance)
        if (!add({from.row + DIRECTIONS[d][0] * distance,
                  from.column + DIRECTIONS[d][1] * distance})) break;
}

std::vector<ChessMove> ChessGame::pseudoMoves(const Board& board, PieceColor color) const {
    std::vector<ChessMove> result;
    for (int row = 0; row < 8; ++row) for (int column = 0; column < 8; ++column)
        if (board[row][column].type != PieceType::None && board[row][column].color == color)
            appendPieceMoves(board, {row, column}, result);
    return result;
}

ChessGame::Board ChessGame::applied(const Board& board, const ChessMove& move) const {
    Board result = board;
    Piece piece = result[move.from.row][move.from.column];
    result[move.from.row][move.from.column] = EMPTY;
    if (move.enPassant) result[move.from.row][move.to.column] = EMPTY;
    if (move.castle) {
        const int rookFrom = move.to.column == 6 ? 7 : 0;
        const int rookTo = move.to.column == 6 ? 5 : 3;
        result[move.to.row][rookTo] = result[move.to.row][rookFrom];
        result[move.to.row][rookFrom] = EMPTY;
    }
    if (piece.type == PieceType::Pawn && (move.to.row == 0 || move.to.row == 7))
        piece.type = move.promotion == PieceType::None ? PieceType::Queen : move.promotion;
    result[move.to.row][move.to.column] = piece;
    return result;
}

std::vector<ChessMove> ChessGame::legalMoves() const {
    std::vector<ChessMove> result;
    if (status_ == GameStatus::Checkmate || status_ == GameStatus::Stalemate || status_ == GameStatus::Draw)
        return result;
    for (const ChessMove& move : pseudoMoves(board_, turn_))
        if (!inCheck(applied(board_, move), turn_)) result.push_back(move);
    return result;
}

std::vector<ChessMove> ChessGame::legalMovesFrom(Square square) const {
    std::vector<ChessMove> result;
    for (const ChessMove& move : legalMoves()) if (move.from == square) result.push_back(move);
    return result;
}

bool ChessGame::move(const ChessMove& requested) {
    const auto legal = legalMoves();
    const auto found = std::find_if(legal.begin(), legal.end(), [&](const ChessMove& candidate) {
        return candidate.from == requested.from && candidate.to == requested.to;
    });
    if (found == legal.end()) return false;
    ChessMove move = *found;
    move.promotion = requested.promotion;
    const Piece moving = board_[move.from.row][move.from.column];
    const Piece captured = board_[move.to.row][move.to.column];
    board_ = applied(board_, move);
    if (moving.type == PieceType::King) {
        if (moving.color == PieceColor::White) whiteKingCastle_ = whiteQueenCastle_ = false;
        else blackKingCastle_ = blackQueenCastle_ = false;
    }
    auto disableRookRight = [&](Square square) {
        if (square == Square{7,0}) whiteQueenCastle_ = false;
        if (square == Square{7,7}) whiteKingCastle_ = false;
        if (square == Square{0,0}) blackQueenCastle_ = false;
        if (square == Square{0,7}) blackKingCastle_ = false;
    };
    if (moving.type == PieceType::Rook) disableRookRight(move.from);
    if (captured.type == PieceType::Rook) disableRookRight(move.to);
    enPassantTarget_.reset();
    if (moving.type == PieceType::Pawn && std::abs(move.to.row - move.from.row) == 2)
        enPassantTarget_ = Square{(move.from.row + move.to.row) / 2, move.from.column};
    halfmoveClock_ = moving.type == PieceType::Pawn || captured.type != PieceType::None ? 0 : halfmoveClock_ + 1;
    lastMove_ = move;
    turn_ = opposite(turn_);
    updateStatus();
    return true;
}

void ChessGame::updateStatus() {
    const bool check = inCheck(board_, turn_);
    const bool noMoves = legalMoves().empty();
    if (noMoves) status_ = check ? GameStatus::Checkmate : GameStatus::Stalemate;
    else if (halfmoveClock_ >= 100) status_ = GameStatus::Draw;
    else status_ = check ? GameStatus::Check : GameStatus::Playing;
}
