#include "game.hpp"

SE_UI *ctx;
SE_String input_text;

Game::Game(SDL_Window *window) {
        //- window and viewport
    this->window = window;
    i32 window_w;
    i32 window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);
    Rect viewport = {0, 0, (f32)window_w, (f32)window_h};

    ctx = NEW(SE_UI);
    seui_init(ctx, &input, viewport, -1000, 1000);
    sestring_init(&input_text, "");
}

Game::~Game() {
    free(ctx);
}

void Game::update(f32 delta_time) {
    if (seui_panel(ctx, "test")) {
        seui_panel_row(ctx, 32, 3);

        seui_label(ctx, "text");

        if (seui_button(ctx, "button")) {
            printf("pressed button\n");
        }

        seui_input_text(ctx, &input_text);
    }
}

void Game::render() {

        //- UI
    glClear(GL_DEPTH_BUFFER_BIT);
    seui_render(ctx);
}