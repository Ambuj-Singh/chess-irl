#pragma once

#include <SDL3/SDL.h>

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

    void renderChessPlaceholder();
    void renderLudoPlaceholder();

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;

    AppScreen currentScreen_ = AppScreen::Dashboard;
    bool running_ = false;
};