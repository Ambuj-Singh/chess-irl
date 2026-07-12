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

    if (!chessRenderer_.initialize(renderer_)) {
        std::cerr << "Chess renderer initialization failed\n";
        shutdown();
        return false;
    }

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

        if (
            currentScreen_ != AppScreen::Dashboard &&
            backButtonClicked(event)
        ) {
            currentScreen_ = AppScreen::Dashboard;
            continue;
        }

        switch (currentScreen_) {
            case AppScreen::Dashboard:
                currentScreen_ =
                    Dashboard::handleEvent(event);
                break;

            case AppScreen::Chess:
                chessRenderer_.handleEvent(event, renderer_);
                break;

            case AppScreen::Ludo:
                // Ludo events will be forwarded here later.
                break;
        }
    }
}

void App::update() {
    if (currentScreen_ == AppScreen::Chess) {
        chessRenderer_.update();
    }
}

void App::render() {
    switch (currentScreen_) {
        case AppScreen::Dashboard:
            Dashboard::render(renderer_);
            break;

        case AppScreen::Chess:
            chessRenderer_.render(renderer_);
            renderBackButton();
            break;

        case AppScreen::Ludo:
            renderLudoPlaceholder();
            renderBackButton();
            break;
    }

    SDL_RenderPresent(renderer_);
}

bool App::backButtonClicked(const SDL_Event& event) const {
    if (
        event.type != SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.button.button != SDL_BUTTON_LEFT
    ) {
        return false;
    }

    const SDL_FRect button = backButtonRect();
    return event.button.x >= button.x &&
           event.button.x <= button.x + button.w &&
           event.button.y >= button.y &&
           event.button.y <= button.y + button.h;
}

SDL_FRect App::backButtonRect() const {
    int width = 0;
    int height = 0;
    SDL_GetRenderOutputSize(renderer_, &width, &height);

    const float margin = 20.0F;
    const float buttonWidth = 92.0F;
    const float buttonHeight = 46.0F;

    return {
        margin,
        margin,
        buttonWidth,
        buttonHeight
    };
}

void App::renderBackButton() const {
    const SDL_FRect button = backButtonRect();

    float mouseX = 0.0F;
    float mouseY = 0.0F;
    SDL_GetMouseState(&mouseX, &mouseY);

    const bool hovered =
        mouseX >= button.x &&
        mouseX <= button.x + button.w &&
        mouseY >= button.y &&
        mouseY <= button.y + button.h;

    if (hovered) {
        SDL_SetRenderDrawColor(renderer_, 75, 84, 110, 255);
    } else {
        SDL_SetRenderDrawColor(renderer_, 42, 48, 65, 255);
    }
    SDL_RenderFillRect(renderer_, &button);

    SDL_SetRenderDrawColor(renderer_, 177, 187, 220, 255);
    SDL_RenderRect(renderer_, &button);

    // Back arrow, drawn directly so the button does not require a font asset.
    const float centerY = button.y + button.h / 2.0F;
    const SDL_FRect shaft{
        button.x + 30.0F,
        centerY - 2.5F,
        36.0F,
        5.0F
    };
    SDL_RenderFillRect(renderer_, &shaft);

    SDL_Vertex arrow[] = {
        {{button.x + 20.0F, centerY}, {177, 187, 220, 255}, {0.0F, 0.0F}},
        {{button.x + 34.0F, centerY - 12.0F}, {177, 187, 220, 255}, {0.0F, 0.0F}},
        {{button.x + 34.0F, centerY + 12.0F}, {177, 187, 220, 255}, {0.0F, 0.0F}}
    };
    SDL_RenderGeometry(renderer_, nullptr, arrow, 3, nullptr, 0);
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
    chessRenderer_.shutdown();

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
