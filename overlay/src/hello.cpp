#include <iostream>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#define EXIT_FAILURE_CREATE_WINDOW 1

static std::string asset_path(const std::string& rel) {
    const char* base = SDL_GetBasePath();
    return std::string(base ? base : "") + rel;
}

int main(int argc, char *argv[]) {
    // Initialize libraries
    if (!SDL_Init(SDL_INIT_VIDEO)) { std::cerr << SDL_GetError() << "\n"; return 1; }
    if (!TTF_Init())               { std::cerr << SDL_GetError() << "\n"; return 1; }

    // Initialize window and loop variables
    SDL_Window *window = nullptr;
    bool done = false;
    int width = 640, height = 360;

    // Create window
    // Reference for SDL_CreateWindow: https://wiki.libsdl.org/SDL3/SDL_CreateWindow
    window = SDL_CreateWindow(
        "Hello World!",
        width,
        height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT
        // SDL_WINDOW_NOT_FOCUSABLE
    );

    // Create window error handling
    if (window == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return EXIT_FAILURE_CREATE_WINDOW;
    }

    // Initialize graphical surfaces
    // Reference for SDL_GetWindowSurface: https://wiki.libsdl.org/SDL3/SDL_GetWindowSurface
    SDL_Surface *surface = surface = SDL_GetWindowSurface(window);
    // Reference for TTF_CreateSurfaceTextEngine: https://wiki.libsdl.org/SDL3_ttf/TTF_CreateSurfaceTextEngine
    TTF_TextEngine *textEngine = TTF_CreateSurfaceTextEngine();
    std::string fontPath = asset_path("assets/courier.ttf");
    TTF_Font *ttfFont = TTF_OpenFont(fontPath.c_str(), 32.0f);

    if (surface == nullptr) {
        std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (textEngine == nullptr) {
        std::cerr << "Failed to load text engine: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (ttfFont == nullptr) {
        std::cerr << "Failed to open " << fontPath << ": " << SDL_GetError() << std::endl;
        return 1;
    }

    // Reference for TTF_CreateText: https://wiki.libsdl.org/SDL3_ttf/TTF_CreateText
    std::string textRaw = std::to_string(width) + ", " + std::to_string(height);
    TTF_Text *text = TTF_CreateText(
        textEngine,
        ttfFont,
        textRaw.c_str(),
        textRaw.length()
    );
    
    surface = SDL_GetWindowSurface(window);
    if (!surface) { std::cerr << SDL_GetError() << "\n"; return 1; }
    SDL_ClearSurface(surface, 0, 0, 0, 0);
    TTF_DrawSurfaceText(text, 32, 32, surface);
    SDL_UpdateWindowSurface(window);

    // Main loop
    SDL_Event event;
    while (!done && SDL_WaitEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            done = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            // Of type Sint32
            width = event.window.data1;
            height = event.window.data2;

            // Reference for TTF_CreateText: https://wiki.libsdl.org/SDL3_ttf/TTF_CreateText
            textRaw = std::to_string(width) + ", " + std::to_string(height);
            if (!TTF_SetTextString(
                    text, 
                    textRaw.c_str(), 
                    textRaw.length())
                ) {
                std::cerr << "Failed to create text: " << SDL_GetError() << std::endl;
                return 1;
            }

            surface = SDL_GetWindowSurface(window);
            if (!surface) { std::cerr << SDL_GetError() << "\n"; return 1; }
            SDL_ClearSurface(surface, 0, 0, 0, 0);
            // Reference for TTF_DrawSurfaceText: https://wiki.libsdl.org/SDL3_ttf/TTF_DrawSurfaceText
            TTF_DrawSurfaceText(text, 32, 32, surface);

            SDL_UpdateWindowSurface(window);

            break;
        default:
            break;
        }
    }

    // Clean up gracefully
    TTF_CloseFont(ttfFont);
    TTF_DestroySurfaceTextEngine(textEngine);
    TTF_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}