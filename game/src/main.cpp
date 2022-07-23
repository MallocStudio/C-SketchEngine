#define SDL_MAIN_HANDLED // gets rid of linking errors

#include "GL/glew.h"
#include "game.hpp"
#include "stdio.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

int main() {
    SDL_Window *window;
    // i32 window_w = 1600;
    // i32 window_h = 1024;
    i32 window_w = 1920;
    i32 window_h = 1080;
        //- init SDL
    ERROR_ON_NOTZERO_SDL(SDL_Init(SDL_INIT_EVERYTHING), "init_sdl");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        //- Create a window
    window = SDL_CreateWindow("SketchEngine_OpenGL Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    ERROR_ON_NULL_SDL(window, "window was null");

        //- Create context
    SDL_GLContext g_context = SDL_GL_CreateContext(window);
    ERROR_ON_NULL_SDL(g_context, "OpenGL context could not be created!");

        //- Init GLEW
    GLenum glew_error = glewInit();
    if (glew_error != GLEW_OK) {
        printf("ERROR init GLEW! %s\n", glewGetErrorString(glew_error));
    }

        //- Init IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplSDL2_InitForOpenGL(window, g_context);
    ImGui_ImplOpenGL3_Init("#version 450");
    ImGui::StyleColorsDark();
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        //- Use Vsync
    ERROR_ON_NOTZERO_SDL(SDL_GL_SetSwapInterval(1), "Warning: Unable to set VSync");

    App *game = new App(window);

    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

        //- Enable dropping of files
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    char *dropped_file_dir;

        //- delta time
    Uint64 now  = SDL_GetPerformanceCounter();
    Uint64 last = 0;
    f64 delta_time = 0;
    Uint32 ticks = SDL_GetTicks();
    Uint32 initial_tick = ticks;

        //- main loop
    while (!game->should_quit) {
            //- events
        SDL_Event event;
        bool keyboard_down = false;
        bool keyboard_pressed = false;
        game->m_input.mouse_wheel = 0; // reset this to zero
        while (!game->should_quit && SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
                case SDL_QUIT: { // -- wanting to quit
                    game->should_quit = true;
                } break;
                case SDL_WINDOWEVENT: { // -- resized window
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        // printf("window resized\n");
                        // SDL_GetWindowSize(window, &window_w, &window_h);
                        // glViewport(0, 0, window_w, window_h);
                    }
                } break;
                case SDL_MOUSEWHEEL: {
                    game->m_input.mouse_wheel = event.wheel.preciseY;
                } break;
                // case SDL_KEYDOWN: {
                //     // keyboard_pressed = true;
                //     // keyboard_down = true;
                // } break;
                // case SDL_TEXTINPUT: { // this event happens after SDL_StartTextInput() is called
                //     if (game->m_input.text_input_stream != NULL) {
                //         if (game->m_input.is_text_input_only_numeric) {
                //             if ((event.text.text[0] >= (i32)'0' && event.text.text[0] <= (i32)'9')
                //                 || event.text.text[0] == (i32)'-' || event.text.text[0] == (i32)'.') {
                //                 se_string_append(game->m_input.text_input_stream, event.text.text);
                //             }
                //         } else {
                //             se_string_append(game->m_input.text_input_stream, event.text.text);
                //             // printf("WHAT!!! %s\n", event.text.text);
                //         }
                //     } else {
                //         printf("Warning: tried to append to input text stream but it was null\n");
                //     }
                // } break;
                // case SDL_TEXTEDITING: {// this event happens after SDL_StartTextInput() is called
                // } break;
                case SDL_DROPFILE: {
                    //- Drop Files
                    dropped_file_dir = event.drop.file;
                        // @temp
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "file dropped on window", dropped_file_dir, window);
                    SDL_free(dropped_file_dir);
                } break;
            }
        }

        //- ImGUI
        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        last = now;
        now = SDL_GetPerformanceCounter();
        delta_time = (f64)((now - last) / (f64)SDL_GetPerformanceFrequency());

        ticks = SDL_GetTicks();
        game->fps = 0.0f;//1000.0f / se_math_max((initial_tick - ticks), 0.000001f);

        game->update((f32)delta_time);
        game->render();
        game->end_of_frame();

            //- Render
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        // glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // -- exit
    delete game;

    //- ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    SDL_GL_DeleteContext(g_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("exited successfully!\n");
    return 0;
}
