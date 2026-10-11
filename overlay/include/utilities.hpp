#include <iostream>
#include <string>
#include <cstdint>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// debugging
#include <bitset>

#define INITIAL_WINDOW_WIDTH 640
#define INITIAL_WINDOW_HEIGHT 480

#define EXIT_FAILURE_INITIALIZATION     1
#define EXIT_FAILURE_WINDOW_RENDERER    2
#define EXIT_FAILURE_CREATE_SURFACE     3
#define EXIT_FAILURE_LOAD_TEXT_ENGINE   4
#define EXIT_FAILURE_OPEN_FONT_PATH     5

#define LEFT_EDGE       0b0001
#define RIGHT_EDGE      0b0010
#define TOP_EDGE        0b0100
#define BOT_EDGE        0b1000

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLIP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

/**
 * Get indicator of edge that mouse is hovering over within a given area around edge
 *
 * @param frame     The rectangle to check edge proximity with
 * @param threshold The +/- range of acceptable proximity
 * @return          4-bit flag of nearest edge:
 *                  - Left edge is closest: 0001
 *                  - Right edge:           0010
 *                  - Top edge:             0100
 *                  - Bottom edge:          1000
 *                  A 2-way combination of bits indicate corner
 */
uint8_t getEdgeIntersection(SDL_FRect * frame, float grabThreshold);