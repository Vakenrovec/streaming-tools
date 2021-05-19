#include "VideoDisplayProcessor.h"
#include "Logger.h"
#include <thread>
#include <chrono>

VideoDisplayProcessor::VideoDisplayProcessor(int width, int height)
: m_screen(nullptr)
, m_renderer(nullptr)
, m_texture(nullptr)
, m_width(width)
, m_height(height)
{
    m_rect.x = 0;
    m_rect.y = 0;
    m_rect.w = m_width;
    m_rect.h = m_height;
}

void VideoDisplayProcessor::Init()
{
    m_screen = SDL_CreateWindow(
        "SDL Media Player",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        m_width,
        m_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!m_screen)
    {
        LOG_EX_WARN_WITH_CONTEXT("SDL: could not create window");
        return;
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

    DataProcessor::Init();
}

void VideoDisplayProcessor::Destroy()
{
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_screen) {
        SDL_DestroyWindow(m_screen);
        m_screen = nullptr;
    }

    DataProcessor::Destroy();
}

void VideoDisplayProcessor::Process(const media_packet_ptr& pkt)
{
    if (pkt->header.type == media_packet_type_t::YV12)
    {
        Display(pkt);
        DataProcessor::Process(pkt);
    }
    else
    {
        LOG_EX_WARN_WITH_CONTEXT("Incorrect packet type: %d", pkt->header.type);
    }
}

void VideoDisplayProcessor::Display(const media_packet_ptr& pkt)
{
    int ret = SDL_UpdateYUVTexture(
        m_texture,                                      // the texture to update
        &m_rect,                                        // a pointer to the rectangle of pixels to update, or NULL to update the entire texture
        pkt->data,                                      // the raw pixel data for the Y plane
        m_width,                                        // the number of bytes between rows of pixel data for the Y plane
        pkt->data + m_width * m_height,                 // the raw pixel data for the U plane
        m_width / 2,                                    // the number of bytes between rows of pixel data for the U plane
        pkt->data + m_width * m_height * 5 / 4,         // the raw pixel data for the V plane
        m_width / 2                                     // the number of bytes between rows of pixel data for the V plane
    );

    ret = SDL_RenderClear(m_renderer);

    ret = SDL_RenderCopy(
        m_renderer,     // the rendering context
        m_texture,      // the source texture
        nullptr,           // the source SDL_Rect structure or NULL for the entire texture
        nullptr            // the destination SDL_Rect structure or NULL for the entire rendering
                        // target; the texture will be stretched to fill the given rectangle
    );

    // update the screen with any rendering performed since the previous call
    SDL_RenderPresent(m_renderer);
}
