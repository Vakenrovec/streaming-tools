#include "VideoDisplay.h"

VideoDisplay::VideoDisplay()
{
    m_screen = SDL_CreateWindow(
        "FFmpeg SDL Video Player",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        m_width,
        m_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!m_screen)
    {
        printf("SDL: could not create window - exiting.\n");
    }
    SDL_GL_SetSwapInterval(1);
    m_renderer = SDL_CreateRenderer(m_screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    m_texture = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_YV12,
        SDL_TEXTUREACCESS_STREAMING,
        m_width,
        m_height
    );
}
    
void VideoDisplay::Display(Image& yv12Image)
{
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = m_width;
    rect.h = m_height;

    int ret = SDL_UpdateYUVTexture(
        m_texture,            // the texture to update
        &rect,              // a pointer to the rectangle of pixels to update, or NULL to update the entire texture
        yv12Image.data,      // the raw pixel data for the Y plane
        1280,  // the number of bytes between rows of pixel data for the Y plane
        yv12Image.data + m_width * m_height,      // the raw pixel data for the U plane
        640,  // the number of bytes between rows of pixel data for the U plane
        yv12Image.data + m_width * m_height * 5 / 4,      // the raw pixel data for the V plane
        640   // the number of bytes between rows of pixel data for the V plane
    );

    ret = SDL_RenderClear(m_renderer);

    ret = SDL_RenderCopy(
        m_renderer,   // the rendering context
        m_texture,    // the source texture
        NULL,       // the source SDL_Rect structure or NULL for the entire texture
        NULL        // the destination SDL_Rect structure or NULL for the entire rendering
                    // target; the texture will be stretched to fill the given rectangle
    );

    // update the screen with any rendering performed since the previous call
    SDL_RenderPresent(m_renderer);
}
