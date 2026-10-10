#include <iostream>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#define INITIAL_WINDOW_WIDTH 640
#define INITIAL_WINDOW_HEIGHT 480

#define EXIT_FAILURE_INITIALIZATION     1
#define EXIT_FAILURE_WINDOW_RENDERER    2
#define EXIT_FAILURE_CREATE_SURFACE     3
#define EXIT_FAILURE_LOAD_TEXT_ENGINE   4
#define EXIT_FAILURE_OPEN_FONT_PATH     5

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLIP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

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
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize video: %s\n", SDL_GetError());
        return EXIT_FAILURE_INITIALIZATION;
    }
    if (!TTF_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize TTF: %s\n", SDL_GetError());
        return EXIT_FAILURE_INITIALIZATION;
    }

    // Initialize window variables
    SDL_Window *window = nullptr;
    int windowWidth = INITIAL_WINDOW_WIDTH, windowHeight = INITIAL_WINDOW_HEIGHT;
    SDL_Renderer *renderer = nullptr;

    // Create window and renderer
    if (!SDL_CreateWindowAndRenderer("宝可梦学习者",
                                    windowWidth,
                                    windowHeight,
                                    SDL_WINDOW_RESIZABLE |
                                        SDL_WINDOW_ALWAYS_ON_TOP |
                                        SDL_WINDOW_TRANSPARENT,
                                        // SDL_WINDOW_NOT_FOCUSABLE
                                    &window,
                                    &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't create window/renderer: %s", SDL_GetError());
        return EXIT_FAILURE_WINDOW_RENDERER;
    }
    SDL_SetRenderLogicalPresentation(renderer, windowWidth, windowHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Initialize graphical surfaces
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    TTF_TextEngine *textEngine = TTF_CreateSurfaceTextEngine();
    std::string fontPath = asset_path("assets/courier.ttf");
    TTF_Font *ttfFont = TTF_OpenFont(fontPath.c_str(), 32.0f);
    SDL_FRect textBoxFrame;
    SDL_FRect popupFrame;

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
    
    textBoxFrame.x = textBoxFrame.y = 10; textBoxFrame.x = textBoxFrame.y = 10;
    textBoxFrame.w = textBoxFrame.h = 100 + (100 * 1);
    popupFrame.x = popupFrame.y = 10; popupFrame.x = popupFrame.y = 10;
    popupFrame.w = popupFrame.h = 100 + (100 * 1);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);  /* red, full alpha */

    // Create text artifact from window dimensions
    std::string textRaw = std::to_string((int)textBoxFrame.w) + ", " + std::to_string((int)textBoxFrame.h);
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
     * @param textRaw The string to render.
     */
    auto renderText = [&renderer, &text](SDL_Surface *surface, std::string textRaw) -> void {
        if (!TTF_SetTextString(
            text, 
            textRaw.c_str(), 
            textRaw.length())
        ) {
            std::cerr << "Failed to create text: " << SDL_GetError() << std::endl;
            return;
        }

        SDL_ClearSurface(surface, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
        TTF_DrawSurfaceText(text, 32, 32, surface);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
    };
    
    SDL_RenderRect(renderer, &textBoxFrame);
    renderText(surface, textRaw);

    SDL_RenderPresent(renderer);

    // Main loop
    SDL_Event event;
    bool done = false;
    int invertControls = 1;
    while (!done && SDL_WaitEvent(&event)) {

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);  /* clear */
        SDL_RenderClear(renderer);
        
        switch (event.type) {
        case SDL_EVENT_QUIT:            // Quit the game
            done = true;
            break;

        case SDL_EVENT_WINDOW_RESIZED:  // Type new window dimensions
            // Of type Sint32
            windowWidth = event.window.data1;
            windowHeight = event.window.data2;
            break;

        case SDL_EVENT_KEY_DOWN:        // Key down event
            if (event.key.key == SDLK_SPACE) {
                invertControls = -1;
            }
            switch (event.key.key) {
                case SDLK_ESCAPE:
                    done = true;
                    break;
                case SDLK_L:
                    textBoxFrame.w = MAX(0, textBoxFrame.w + (10 * invertControls));
                    break;
                case SDLK_H:
                    textBoxFrame.x = textBoxFrame.w > 0 || invertControls < 0 ? textBoxFrame.x + (10 * invertControls) : textBoxFrame.x;
                    textBoxFrame.w = MAX(0, textBoxFrame.w - (10 * invertControls));
                    break;
                case SDLK_J:
                    textBoxFrame.h = MAX(0, textBoxFrame.h + (10 * invertControls));
                    break;
                case SDLK_K:
                    textBoxFrame.y = textBoxFrame.h > 0 || invertControls < 0 ? textBoxFrame.y + (10 * invertControls) : textBoxFrame.y;
                    textBoxFrame.h = MAX(0, textBoxFrame.h - (10 * invertControls));
                    break;
            }
            break;

        case SDL_EVENT_KEY_UP:        // Key up event
            if (event.key.key == SDLK_SPACE) {
                invertControls = 1;
            }
            break;
        }
        
        float mouseX = -1.f, mouseY = -1.f;
        SDL_MouseButtonFlags mouseButtons = SDL_GetMouseState( &mouseX, &mouseY );
        if (mouseButtons & SDL_BUTTON_LMASK) {
            popupFrame.x = CLIP(mouseX, 0, windowWidth-1);
            popupFrame.y = CLIP(mouseY, 0, windowHeight-1);
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);  /* red, full alpha */
        textRaw = std::to_string((int)textBoxFrame.w) + ", " + std::to_string((int)textBoxFrame.h);
        renderText(surface, textRaw);
        SDL_RenderRect(renderer, &textBoxFrame);
        SDL_RenderRect(renderer, &popupFrame);
        SDL_RenderPresent(renderer);
    }

    // Clean up gracefully
    TTF_CloseFont(ttfFont);
    TTF_DestroySurfaceTextEngine(textEngine);
    TTF_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}