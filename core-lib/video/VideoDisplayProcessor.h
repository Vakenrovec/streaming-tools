#pragma once

#include "DataProcessor.h"
#include "MediaPacket.h"
#include <SDL2/SDL.h>

class VideoDisplayProcessor: public DataProcessor
{
public:
    VideoDisplayProcessor(int width, int height);

    void Init() override;
    void Destroy() override;

    void Process(const media_packet_ptr& pkt) override;

private:
    void Display(const media_packet_ptr& pkt);

    SDL_Window *m_window;
    SDL_Renderer *m_renderer;
    SDL_Texture *m_texture;
    SDL_Rect m_srcRect, m_dstRect;
    const int m_width;
    const int m_height;
    const SDL_RendererFlip m_flip = SDL_RendererFlip::SDL_FLIP_HORIZONTAL;
};
