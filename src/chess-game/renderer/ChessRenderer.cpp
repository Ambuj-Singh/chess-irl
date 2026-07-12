#include "ChessRenderer.hpp"

#include <SDL3_image/SDL_image.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {
SDL_FRect spriteCell(PieceType piece) {
    int index = 0;
    switch (piece) {
        case PieceType::Bishop: index = 1; break;
        case PieceType::Knight: index = 4; break;
        case PieceType::King: index = 8; break;
        case PieceType::Queen: index = 9; break;
        case PieceType::Rook: index = 11; break;
        case PieceType::Pawn: index = 12; break;
        default: break;
    }
    return {float((index % 4) * 128), float((index / 4) * 128), 128.0F, 128.0F};
}

SDL_Texture* loadSheet(SDL_Renderer* renderer, const std::string& path,
                       Uint8 red, Uint8 green, Uint8 blue) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Could not load " << path << ": " << SDL_GetError() << '\n';
        return nullptr;
    }
    SDL_SetSurfaceColorKey(surface, true, SDL_MapSurfaceRGB(surface, red, green, blue));
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    if (texture) SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    return texture;
}

bool contains(const SDL_FRect& rect, float x, float y) {
    return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
}

void fill(SDL_Renderer* renderer, const SDL_FRect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
}

void label(SDL_Renderer* renderer, float x, float y, const char* text,
           Uint8 r = 230, Uint8 g = 234, Uint8 b = 242, float scale = 2.0F) {
    float previousX = 1.0F;
    float previousY = 1.0F;
    SDL_GetRenderScale(renderer, &previousX, &previousY);
    SDL_SetRenderScale(renderer, scale, scale);
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDebugText(renderer, x / scale, y / scale, text);
    SDL_SetRenderScale(renderer, previousX, previousY);
}

const char* statusText(const ChessGame& game) {
    switch (game.status()) {
        case GameStatus::Check: return "CHECK";
        case GameStatus::Checkmate: return game.turn() == PieceColor::White ? "BLACK WINS" : "WHITE WINS";
        case GameStatus::Stalemate: return "STALEMATE";
        case GameStatus::Draw: return "DRAW";
        default: return game.turn() == PieceColor::White ? "WHITE TO MOVE" : "BLACK TO MOVE";
    }
}
}

bool ChessRenderer::initialize(SDL_Renderer* renderer) {
    const std::string root = std::string(CHESSIRL_ASSET_DIR) + "/chess-game/pieces/Top Down/Pieces/";
    whitePieces_ = loadSheet(renderer, root + "White/White - Marble 1 128x128.png", 0, 0, 0);
    blackPieces_ = loadSheet(renderer, root + "Black/Black - Marble 1 128x128.png", 0, 128, 128);
    const std::string boardPath = std::string(CHESSIRL_ASSET_DIR) +
        "/chess-game/boards/rect-8x8.svg";
    boardTexture_ = IMG_LoadTexture(renderer, boardPath.c_str());
    if (!boardTexture_) {
        std::cerr << "Could not load " << boardPath << ": " << SDL_GetError() << '\n';
    } else {
        SDL_SetTextureScaleMode(boardTexture_, SDL_SCALEMODE_LINEAR);
    }
    return whitePieces_ && blackPieces_ && boardTexture_;
}

SDL_FRect ChessRenderer::boardRect(SDL_Renderer* renderer) {
    int width = 0, height = 0;
    SDL_GetRenderOutputSize(renderer, &width, &height);
    constexpr float sidebarWidth = 250.0F;
    constexpr float contentMargin = 36.0F;
    const float availableWidth = float(width) - sidebarWidth - contentMargin * 2.0F;
    const float availableHeight = float(height) - contentMargin * 2.0F;
    const float outerSize = std::max(245.0F, std::min(availableWidth, availableHeight));
    const float contentWidth = float(width) - sidebarWidth;
    const float outerX = sidebarWidth + (contentWidth - outerSize) / 2.0F;
    const float outerY = (height - outerSize) / 2.0F;
    // rect-8x8.svg has an 8px border around a 768px playing grid.
    const float inset = outerSize * (8.0F / 784.0F);
    const float playSize = outerSize * (768.0F / 784.0F);
    return {outerX + inset, outerY + inset, playSize, playSize};
}

void ChessRenderer::startGame(bool versusCpu) {
    game_.reset();
    versusCpu_ = versusCpu;
    selected_.reset();
    selectedMoves_.clear();
    cpuMoveDue_ = 0;
    view_ = View::Game;
}

void ChessRenderer::handleEvent(const SDL_Event& event, SDL_Renderer* renderer) {
    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN || event.button.button != SDL_BUTTON_LEFT) return;
    int width = 0, height = 0;
    SDL_GetRenderOutputSize(renderer, &width, &height);
    if (view_ == View::ModeSelect) {
        const float menuX = std::max(170.0F, width * 0.21F);
        const float menuY = height * 0.57F;
        const SDL_FRect cpu{menuX, menuY, 330.0F, 54.0F};
        const SDL_FRect local{menuX, menuY + 70.0F, 330.0F, 54.0F};
        if (contains(cpu, event.button.x, event.button.y)) startGame(true);
        else if (contains(local, event.button.x, event.button.y)) startGame(false);
        return;
    }

    const SDL_FRect board = boardRect(renderer);
    const SDL_FRect restart{20.0F, 92.0F, 210.0F, 42.0F};
    const SDL_FRect modes{20.0F, 146.0F, 210.0F, 42.0F};
    if (contains(restart, event.button.x, event.button.y)) { startGame(versusCpu_); return; }
    if (contains(modes, event.button.x, event.button.y)) { view_ = View::ModeSelect; return; }
    if (!contains(board, event.button.x, event.button.y) ||
        (versusCpu_ && game_.turn() == PieceColor::Black)) return;

    const float squareSize = board.w / 8.0F;
    const Square clicked{int((event.button.y - board.y) / squareSize),
                         int((event.button.x - board.x) / squareSize)};
    const auto destination = std::find_if(selectedMoves_.begin(), selectedMoves_.end(),
        [&](const ChessMove& move) { return move.to == clicked; });
    if (destination != selectedMoves_.end()) {
        game_.move(*destination);
        selected_.reset();
        selectedMoves_.clear();
        if (versusCpu_ && game_.turn() == PieceColor::Black) cpuMoveDue_ = SDL_GetTicks() + 350;
        return;
    }
    const Piece piece = game_.pieceAt(clicked);
    if (piece.type != PieceType::None && piece.color == game_.turn()) {
        selected_ = clicked;
        selectedMoves_ = game_.legalMovesFrom(clicked);
    } else {
        selected_.reset();
        selectedMoves_.clear();
    }
}

void ChessRenderer::update() {
    if (!versusCpu_ || view_ != View::Game || game_.turn() != PieceColor::Black ||
        cpuMoveDue_ == 0 || SDL_GetTicks() < cpuMoveDue_) return;
    auto moves = game_.legalMoves();
    if (moves.empty()) return;
    int bestScore = -1;
    std::vector<ChessMove> best;
    for (const auto& move : moves) {
        const Piece captured = game_.pieceAt(move.to);
        int score = 0;
        switch (captured.type) {
            case PieceType::Pawn: score = 1; break;
            case PieceType::Knight: case PieceType::Bishop: score = 3; break;
            case PieceType::Rook: score = 5; break;
            case PieceType::Queen: score = 9; break;
            default: break;
        }
        if (score > bestScore) { bestScore = score; best.clear(); }
        if (score == bestScore) best.push_back(move);
    }
    game_.move(best[std::rand() % best.size()]);
    cpuMoveDue_ = 0;
}

void ChessRenderer::render(SDL_Renderer* renderer) const {
    if (view_ == View::ModeSelect) renderModeSelect(renderer);
    else renderGame(renderer);
}

void ChessRenderer::renderModeSelect(SDL_Renderer* renderer) const {
    int width = 0, height = 0;
    SDL_GetRenderOutputSize(renderer, &width, &height);
    SDL_SetRenderDrawColor(renderer, 2, 2, 3, 255);
    SDL_RenderClear(renderer);

    // Subtle scan lines give the menu the CRT/arcade character of the reference.
    for (int y = 0; y < height; y += 5) {
        fill(renderer, {0.0F, float(y), float(width), 1.0F}, 9, 10, 12, 255);
    }

    const float titleX = std::max(155.0F, width * 0.18F);
    const float titleY = std::max(82.0F, height * 0.15F);
    label(renderer, titleX + 5.0F, titleY + 6.0F, "CHESS", 72, 72, 78, 7.0F);
    label(renderer, titleX, titleY, "CHESS", 224, 224, 218, 7.0F);
    fill(renderer, {titleX - 12.0F, titleY + 76.0F, 340.0F, 7.0F}, 226, 47, 38);
    fill(renderer, {titleX + 42.0F, titleY + 88.0F, 286.0F, 7.0F}, 245, 157, 25);
    label(renderer, titleX, titleY + 126.0F, "PLAY SELECT", 154, 156, 160, 3.0F);

    const float menuX = std::max(170.0F, width * 0.21F);
    const float menuY = height * 0.57F;
    const SDL_FRect cpu{menuX, menuY, 330.0F, 54.0F};
    const SDL_FRect local{menuX, menuY + 70.0F, 330.0F, 54.0F};
    float mx = 0.0F, my = 0.0F;
    SDL_GetMouseState(&mx, &my);
    const bool cpuSelected = contains(cpu, mx, my);
    const bool localSelected = contains(local, mx, my);

    if (cpuSelected) fill(renderer, cpu, 30, 30, 32);
    if (localSelected) fill(renderer, local, 30, 30, 32);
    const float cursorY = localSelected ? local.y + 14.0F : cpu.y + 14.0F;
    SDL_Vertex cursor[] = {
        {{menuX - 42.0F, cursorY}, {255, 213, 23, 255}, {0,0}},
        {{menuX - 42.0F, cursorY + 26.0F}, {255, 213, 23, 255}, {0,0}},
        {{menuX - 15.0F, cursorY + 13.0F}, {255, 213, 23, 255}, {0,0}}
    };
    SDL_RenderGeometry(renderer, nullptr, cursor, 3, nullptr, 0);
    label(renderer, cpu.x + 12.0F, cpu.y + 11.0F, "1 PLAYER", cpuSelected ? 255 : 174,
          cpuSelected ? 225 : 176, cpuSelected ? 42 : 180, 3.5F);
    label(renderer, local.x + 12.0F, local.y + 11.0F, "2 PLAYERS", localSelected ? 255 : 174,
          localSelected ? 225 : 176, localSelected ? 42 : 180, 3.5F);
    label(renderer, menuX, menuY + 151.0F, "1 PLAYER: VS CPU", 95, 99, 107, 1.75F);
    label(renderer, menuX, menuY + 178.0F, "2 PLAYERS: LOCAL", 95, 99, 107, 1.75F);

    // Use the supplied chess art as the menu's large character illustration.
    const float heroSize = std::min(330.0F, height * 0.43F);
    const float heroX = std::min(width - heroSize - 42.0F, width * 0.68F);
    const float heroY = height - heroSize - 32.0F;
    const SDL_FRect queenSource = spriteCell(PieceType::Queen);
    const SDL_FRect knightSource = spriteCell(PieceType::Knight);
    const SDL_FRect knightDest{heroX - heroSize * 0.34F, heroY + heroSize * 0.20F,
                               heroSize * 0.72F, heroSize * 0.72F};
    const SDL_FRect queenDest{heroX, heroY, heroSize, heroSize};
    SDL_RenderTexture(renderer, blackPieces_, &knightSource, &knightDest);
    SDL_RenderTexture(renderer, whitePieces_, &queenSource, &queenDest);

    label(renderer, width - 205.0F, height - 24.0F, "CHESS IRL 2026", 83, 86, 92, 1.5F);
}

void ChessRenderer::renderGame(SDL_Renderer* renderer) const {
    int width = 0, height = 0; SDL_GetRenderOutputSize(renderer, &width, &height);
    SDL_SetRenderDrawColor(renderer, 10, 13, 18, 255); SDL_RenderClear(renderer);
    fill(renderer, {0,0,250.0F,float(height)}, 17,22,31);
    label(renderer, 22, 76, versusCpu_ ? "VS CPU" : "2 PLAYERS", 230,196,126,2.25F);
    fill(renderer, {20,92,210,42}, 46,56,74); label(renderer, 77,105,"RESTART",230,234,242,1.75F);
    fill(renderer, {20,146,210,42}, 46,56,74); label(renderer, 87,159,"MODES",230,234,242,1.75F);
    label(renderer, 22, 218, statusText(game_), game_.status() == GameStatus::Check ? 238 : 200,
          game_.status() == GameStatus::Check ? 90 : 210, 110);

    const SDL_FRect board = boardRect(renderer);
    const float squareSize = board.w / 8.0F;
    const float assetScale = board.w / 768.0F;
    const SDL_FRect boardAsset{board.x - 8.0F * assetScale,
                               board.y - 8.0F * assetScale,
                               784.0F * assetScale,
                               784.0F * assetScale};
    fill(renderer, {boardAsset.x+8,boardAsset.y+10,boardAsset.w,boardAsset.h}, 2,3,5,180);
    SDL_RenderTexture(renderer, boardTexture_, nullptr, &boardAsset);
    for (int row=0; row<8; ++row) for (int col=0; col<8; ++col) {
        SDL_FRect square{board.x+col*squareSize,board.y+row*squareSize,squareSize,squareSize};
        if (game_.lastMove() && (game_.lastMove()->from == Square{row,col} || game_.lastMove()->to == Square{row,col}))
            fill(renderer,square,190,154,42,105);
        if (selected_ && *selected_ == Square{row,col}) fill(renderer,square,65,145,210,130);
    }
    for (const auto& move : selectedMoves_) {
        const float cx=board.x+(move.to.column+.5F)*squareSize, cy=board.y+(move.to.row+.5F)*squareSize;
        const Piece target=game_.pieceAt(move.to);
        if (target.type == PieceType::None) fill(renderer,{cx-squareSize*.10F,cy-squareSize*.10F,squareSize*.20F,squareSize*.20F},45,125,82,190);
        else { SDL_FRect outline{board.x+move.to.column*squareSize+3,board.y+move.to.row*squareSize+3,squareSize-6,squareSize-6}; SDL_SetRenderDrawColor(renderer,184,62,62,255); SDL_RenderRect(renderer,&outline); }
    }
    for (int row=0; row<8; ++row) for (int col=0; col<8; ++col) {
        const Piece piece=game_.pieceAt({row,col}); if (piece.type==PieceType::None) continue;
        SDL_Texture* sheet=piece.color==PieceColor::White ? whitePieces_ : blackPieces_;
        const SDL_FRect source=spriteCell(piece.type);
        const SDL_FRect destination{board.x+col*squareSize,board.y+row*squareSize,squareSize,squareSize};
        SDL_RenderTexture(renderer,sheet,&source,&destination);
    }
}

void ChessRenderer::shutdown() {
    if (whitePieces_) { SDL_DestroyTexture(whitePieces_); whitePieces_=nullptr; }
    if (blackPieces_) { SDL_DestroyTexture(blackPieces_); blackPieces_=nullptr; }
    if (boardTexture_) { SDL_DestroyTexture(boardTexture_); boardTexture_=nullptr; }
}
