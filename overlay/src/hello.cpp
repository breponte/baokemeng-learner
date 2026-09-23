#include <iostream>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#define EXIT_FAILURE_CREATE_WINDOW      1
#define EXIT_FAILURE_CREATE_SURFACE     2
#define EXIT_FAILURE_LOAD_TEXT_ENGINE   3
#define EXIT_FAILURE_OPEN_FONT_PATH     4

/**
 * Resolves a path relative to the executable's base directory.
 *
 * @param rel Path relative to the base directory.
 * @return    The resolved absolute path.
 */
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
    window = SDL_CreateWindow(
        "宝可梦学习者",
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
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    TTF_TextEngine *textEngine = TTF_CreateSurfaceTextEngine();
    std::string fontPath = asset_path("assets/courier.ttf");
    TTF_Font *ttfFont = TTF_OpenFont(fontPath.c_str(), 32.0f);

    // Graphical surfaces error handling
    if (surface == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create surface: %s\n", SDL_GetError());
        return EXIT_FAILURE_CREATE_SURFACE;
    }
    if (textEngine == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load text engine: %s\n", SDL_GetError());
        std::cerr << "" << SDL_GetError() << std::endl;
        return EXIT_FAILURE_LOAD_TEXT_ENGINE;
    }
    if (ttfFont == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to open %s: %s\n", fontPath, SDL_GetError());
        return EXIT_FAILURE_OPEN_FONT_PATH;
    }

    // Create text artifact from window dimensions
    std::string textRaw = std::to_string(width) + ", " + std::to_string(height);
    TTF_Text *text = TTF_CreateText(
        textEngine,
        ttfFont,
        textRaw.c_str(),
        textRaw.length()
    );
    
    /**
     * Renders the given SDL_ttf text onto the window's surface.
     *
     * Retrieves the current window surface, clears it, draws `text` onto it,
     * and presents the updated surface.
     *
     * @param surface Reassigned to the window's current surface (input value discarded).
     * @param text    The prepared TTF_Text object to render.
     */
    auto drawText = [&window](SDL_Surface *surface, TTF_Text *text) -> void {
        surface = SDL_GetWindowSurface(window);
        SDL_ClearSurface(surface, 0, 0, 0, 0);
        TTF_DrawSurfaceText(text, 32, 32, surface);
        SDL_UpdateWindowSurface(window);
    };

    drawText(surface, text);

    // Main loop
    SDL_Event event;
    while (!done && SDL_WaitEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:            // Quit the game
            done = true;
            break;

        case SDL_EVENT_WINDOW_RESIZED:  // Type new window dimensions
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
            drawText(surface, text);
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