#pragma once

#include "../ChessGame.hpp"

#include <SDL3/SDL.h>
#include <optional>
#include <vector>

class ChessRenderer {
public:
    bool initialize(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event, SDL_Renderer* renderer);
    void update();
    void render(SDL_Renderer* renderer) const;
    void shutdown();

private:
    enum class View { ModeSelect, Game };
    static SDL_FRect boardRect(SDL_Renderer* renderer);
    void startGame(bool versusCpu);
    void renderModeSelect(SDL_Renderer* renderer) const;
    void renderGame(SDL_Renderer* renderer) const;

    SDL_Texture* whitePieces_ = nullptr;
    SDL_Texture* blackPieces_ = nullptr;
    SDL_Texture* boardTexture_ = nullptr;
    ChessGame game_;
    View view_ = View::ModeSelect;
    bool versusCpu_ = false;
    std::optional<Square> selected_;
    std::vector<ChessMove> selectedMoves_;
    Uint64 cpuMoveDue_ = 0;
};
