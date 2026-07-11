#pragma once

#include <SDL3/SDL.h>

#include "../core/App.hpp"

class Dashboard {
public:
    static AppScreen handleEvent(
        const SDL_Event& event
    );

    static void render(
        SDL_Renderer* renderer
    );

private:
    static bool containsPoint(
        const SDL_FRect& rectangle,
        float pointX,
        float pointY
    );

    static void renderGameCard(
        SDL_Renderer* renderer,
        const SDL_FRect& card,
        bool hovered,
        bool chessCard
    );

    static void renderChessPreview(
        SDL_Renderer* renderer,
        const SDL_FRect& card
    );

    static void renderLudoPreview(
        SDL_Renderer* renderer,
        const SDL_FRect& card
    );
};