#pragma once

#include "Image.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#ifdef __cplusplus 
}
#endif

class VideoDisplay
{
public:
    VideoDisplay();
    void Display(Image& yv12Image);

private:
    SDL_Window *m_screen;
    SDL_Renderer *m_renderer;
    SDL_Texture *m_texture;
    const int m_width = 1280;
    const int m_height = 720;
};
