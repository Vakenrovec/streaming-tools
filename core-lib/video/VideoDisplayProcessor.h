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

    SDL_Window *m_screen;
    SDL_Renderer *m_renderer;
    SDL_Texture *m_texture;
    SDL_Rect m_rect;
    const int m_width;
    const int m_height;
};
