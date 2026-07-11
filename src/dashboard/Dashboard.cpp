#include "Dashboard.hpp"

namespace {
constexpr SDL_FRect CHESS_CARD{
    180.0F,
    210.0F,
    400.0F,
    300.0F
};

constexpr SDL_FRect LUDO_CARD{
    700.0F,
    210.0F,
    400.0F,
    300.0F
};

void getMousePosition(
    float& mouseX,
    float& mouseY
) {
    SDL_GetMouseState(
        &mouseX,
        &mouseY
    );
}
}

AppScreen Dashboard::handleEvent(
    const SDL_Event& event
) {
    if (
        event.type != SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.button.button != SDL_BUTTON_LEFT
    ) {
        return AppScreen::Dashboard;
    }

    const float mouseX = event.button.x;
    const float mouseY = event.button.y;

    if (
        containsPoint(
            CHESS_CARD,
            mouseX,
            mouseY
        )
    ) {
        return AppScreen::Chess;
    }

    if (
        containsPoint(
            LUDO_CARD,
            mouseX,
            mouseY
        )
    ) {
        return AppScreen::Ludo;
    }

    return AppScreen::Dashboard;
}

bool Dashboard::containsPoint(
    const SDL_FRect& rectangle,
    float pointX,
    float pointY
) {
    return pointX >= rectangle.x &&
           pointX <= rectangle.x + rectangle.w &&
           pointY >= rectangle.y &&
           pointY <= rectangle.y + rectangle.h;
}

void Dashboard::render(
    SDL_Renderer* renderer
) {
    if (renderer == nullptr) {
        return;
    }

    SDL_SetRenderDrawColor(
        renderer,
        14,
        16,
        24,
        255
    );

    SDL_RenderClear(renderer);

    /*
     * Temporary title decoration.
     * We will replace this with SDL3_ttf text later.
     */
    const SDL_FRect titleBar{
        430.0F,
        85.0F,
        420.0F,
        10.0F
    };

    SDL_SetRenderDrawColor(
        renderer,
        100,
        118,
        255,
        255
    );

    SDL_RenderFillRect(
        renderer,
        &titleBar
    );

    float mouseX = 0.0F;
    float mouseY = 0.0F;

    getMousePosition(
        mouseX,
        mouseY
    );

    const bool chessHovered =
        containsPoint(
            CHESS_CARD,
            mouseX,
            mouseY
        );

    const bool ludoHovered =
        containsPoint(
            LUDO_CARD,
            mouseX,
            mouseY
        );

    renderGameCard(
        renderer,
        CHESS_CARD,
        chessHovered,
        true
    );

    renderGameCard(
        renderer,
        LUDO_CARD,
        ludoHovered,
        false
    );
}

void Dashboard::renderGameCard(
    SDL_Renderer* renderer,
    const SDL_FRect& card,
    bool hovered,
    bool chessCard
) {
    if (hovered) {
        SDL_SetRenderDrawColor(
            renderer,
            55,
            61,
            82,
            255
        );
    } else {
        SDL_SetRenderDrawColor(
            renderer,
            34,
            38,
            52,
            255
        );
    }

    SDL_RenderFillRect(
        renderer,
        &card
    );

    if (chessCard) {
        SDL_SetRenderDrawColor(
            renderer,
            98,
            182,
            130,
            255
        );
    } else {
        SDL_SetRenderDrawColor(
            renderer,
            181,
            120,
            232,
            255
        );
    }

    SDL_RenderRect(
        renderer,
        &card
    );

    if (chessCard) {
        renderChessPreview(
            renderer,
            card
        );
    } else {
        renderLudoPreview(
            renderer,
            card
        );
    }

    /*
     * Temporary play-button shape.
     * Text will be added using SDL3_ttf later.
     */
    const SDL_FRect playButton{
        card.x + 110.0F,
        card.y + 242.0F,
        180.0F,
        38.0F
    };

    if (hovered) {
        SDL_SetRenderDrawColor(
            renderer,
            105,
            123,
            255,
            255
        );
    } else {
        SDL_SetRenderDrawColor(
            renderer,
            78,
            90,
            190,
            255
        );
    }

    SDL_RenderFillRect(
        renderer,
        &playButton
    );
}

void Dashboard::renderChessPreview(
    SDL_Renderer* renderer,
    const SDL_FRect& card
) {
    constexpr int GRID_SIZE = 8;
    constexpr float SQUARE_SIZE = 22.0F;

    const float previewSize =
        GRID_SIZE * SQUARE_SIZE;

    const float startX =
        card.x +
        (card.w - previewSize) / 2.0F;

    const float startY =
        card.y + 28.0F;

    for (int row = 0; row < GRID_SIZE; ++row) {
        for (
            int column = 0;
            column < GRID_SIZE;
            ++column
        ) {
            const bool isLight =
                (row + column) % 2 == 0;

            if (isLight) {
                SDL_SetRenderDrawColor(
                    renderer,
                    238,
                    216,
                    181,
                    255
                );
            } else {
                SDL_SetRenderDrawColor(
                    renderer,
                    174,
                    126,
                    91,
                    255
                );
            }

            const SDL_FRect square{
                startX +
                    column * SQUARE_SIZE,

                startY +
                    row * SQUARE_SIZE,

                SQUARE_SIZE,
                SQUARE_SIZE
            };

            SDL_RenderFillRect(
                renderer,
                &square
            );
        }
    }
}

void Dashboard::renderLudoPreview(
    SDL_Renderer* renderer,
    const SDL_FRect& card
) {
    constexpr float PREVIEW_SIZE = 176.0F;

    const float startX =
        card.x +
        (card.w - PREVIEW_SIZE) / 2.0F;

    const float startY =
        card.y + 28.0F;

    const float quadrantSize =
        PREVIEW_SIZE / 2.0F;

    const SDL_FRect redArea{
        startX,
        startY,
        quadrantSize,
        quadrantSize
    };

    const SDL_FRect greenArea{
        startX + quadrantSize,
        startY,
        quadrantSize,
        quadrantSize
    };

    const SDL_FRect blueArea{
        startX,
        startY + quadrantSize,
        quadrantSize,
        quadrantSize
    };

    const SDL_FRect yellowArea{
        startX + quadrantSize,
        startY + quadrantSize,
        quadrantSize,
        quadrantSize
    };

    SDL_SetRenderDrawColor(
        renderer,
        206,
        73,
        73,
        255
    );

    SDL_RenderFillRect(
        renderer,
        &redArea
    );

    SDL_SetRenderDrawColor(
        renderer,
        67,
        165,
        95,
        255
    );

    SDL_RenderFillRect(
        renderer,
        &greenArea
    );

    SDL_SetRenderDrawColor(
        renderer,
        65,
        110,
        204,
        255
    );

    SDL_RenderFillRect(
        renderer,
        &blueArea
    );

    SDL_SetRenderDrawColor(
        renderer,
        230,
        188,
        58,
        255
    );

    SDL_RenderFillRect(
        renderer,
        &yellowArea
    );
}