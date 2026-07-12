#pragma once

#include <SDL3/SDL.h>

#include "chess-game/renderer/ChessRenderer.hpp"

enum class AppScreen {
    Dashboard,
    Chess,
    Ludo
};

class App {
public:
    App() = default;
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    bool initialize();
    void run();

private:
    void handleEvents();
    void update();
    void render();
    void shutdown();

    bool backButtonClicked(const SDL_Event& event) const;
    SDL_FRect backButtonRect() const;
    void renderBackButton() const;

    void renderLudoPlaceholder();

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    ChessRenderer chessRenderer_;

    AppScreen currentScreen_ = AppScreen::Dashboard;
    bool running_ = false;
};
