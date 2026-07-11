#include "App.hpp"

#include "../dashboard/Dashboard.hpp"

#include <iostream>

namespace {
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
}

App::~App() {
    shutdown();
}

bool App::initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr
            << "SDL initialization failed: "
            << SDL_GetError()
            << '\n';

        return false;
    }

    if (!SDL_CreateWindowAndRenderer(
            "Chess IRL",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_RESIZABLE,
            &window_,
            &renderer_
        )) {
        std::cerr
            << "Window or renderer creation failed: "
            << SDL_GetError()
            << '\n';

        shutdown();
        return false;
    }

    running_ = true;
    currentScreen_ = AppScreen::Dashboard;

    return true;
}

void App::run() {
    while (running_) {
        handleEvents();
        update();
        render();
    }
}

void App::handleEvents() {
    SDL_Event event{};

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running_ = false;
            continue;
        }

        /*
         * Escape returns from a game screen to the dashboard.
         * Escape on the dashboard closes the application.
         */
        if (
            event.type == SDL_EVENT_KEY_DOWN &&
            event.key.key == SDLK_ESCAPE
        ) {
            if (currentScreen_ == AppScreen::Dashboard) {
                running_ = false;
            } else {
                currentScreen_ = AppScreen::Dashboard;
            }

            continue;
        }

        switch (currentScreen_) {
            case AppScreen::Dashboard:
                currentScreen_ =
                    Dashboard::handleEvent(event);
                break;

            case AppScreen::Chess:
                // Chess events will be forwarded here later.
                break;

            case AppScreen::Ludo:
                // Ludo events will be forwarded here later.
                break;
        }
    }
}

void App::update() {
    /*
     * Game logic will be updated here later.
     *
     * Example:
     * chessGame_.update();
     * ludoGame_.update();
     */
}

void App::render() {
    switch (currentScreen_) {
        case AppScreen::Dashboard:
            Dashboard::render(renderer_);
            break;

        case AppScreen::Chess:
            renderChessPlaceholder();
            break;

        case AppScreen::Ludo:
            renderLudoPlaceholder();
            break;
    }

    SDL_RenderPresent(renderer_);
}

void App::renderChessPlaceholder() {
    SDL_SetRenderDrawColor(
        renderer_,
        18,
        24,
        22,
        255
    );

    SDL_RenderClear(renderer_);

    constexpr float boardSize = 560.0F;
    constexpr float squareSize = boardSize / 8.0F;

    const float boardX =
        (WINDOW_WIDTH - boardSize) / 2.0F;

    const float boardY =
        (WINDOW_HEIGHT - boardSize) / 2.0F;

    for (int row = 0; row < 8; ++row) {
        for (int column = 0; column < 8; ++column) {
            const bool isLight =
                (row + column) % 2 == 0;

            if (isLight) {
                SDL_SetRenderDrawColor(
                    renderer_,
                    238,
                    216,
                    181,
                    255
                );
            } else {
                SDL_SetRenderDrawColor(
                    renderer_,
                    174,
                    126,
                    91,
                    255
                );
            }

            const SDL_FRect square{
                boardX + column * squareSize,
                boardY + row * squareSize,
                squareSize,
                squareSize
            };

            SDL_RenderFillRect(
                renderer_,
                &square
            );
        }
    }
}

void App::renderLudoPlaceholder() {
    SDL_SetRenderDrawColor(
        renderer_,
        24,
        20,
        34,
        255
    );

    SDL_RenderClear(renderer_);

    const SDL_FRect board{
        360.0F,
        80.0F,
        560.0F,
        560.0F
    };

    SDL_SetRenderDrawColor(
        renderer_,
        215,
        205,
        225,
        255
    );

    SDL_RenderFillRect(
        renderer_,
        &board
    );
}

void App::shutdown() {
    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}