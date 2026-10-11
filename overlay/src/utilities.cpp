#include <utilities.hpp>

uint8_t getEdgeIntersection(SDL_FRect * frame, float grabThreshold) {
    float mouseX = -1.f, mouseY = -1.f;
    SDL_GetMouseState( &mouseX, &mouseY );
    uint8_t mouseoverEdge = 0b000;

    if ((mouseX > frame->x - grabThreshold) &&
            (mouseX < frame->x + grabThreshold)) {

            mouseoverEdge |= LEFT_EDGE;
    }
    if ((mouseX > frame->w + frame->x - grabThreshold) &&
                (mouseX < frame->w + frame->x + grabThreshold)) {

        mouseoverEdge |= RIGHT_EDGE;
    }
    if ((mouseY > frame->y - grabThreshold) &&
            (mouseY < frame->y + grabThreshold)) {
        
        mouseoverEdge |= TOP_EDGE;
    }
    if ((mouseY > frame->h + frame->y - grabThreshold) &&
            (mouseY < frame->h + frame->y + grabThreshold)) {
        
        mouseoverEdge |= BOT_EDGE;
    }

    return mouseoverEdge;
}