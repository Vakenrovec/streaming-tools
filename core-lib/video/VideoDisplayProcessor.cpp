#include "VideoDisplayProcessor.h"
#include "Logger.h"
#include <thread>
#include <chrono>

VideoDisplayProcessor::VideoDisplayProcessor(int width, int height)
: m_window(nullptr)
, m_renderer(nullptr)
, m_texture(nullptr)
, m_width(width)
, m_height(height)
{
    m_srcRect.x = 0;
    m_srcRect.y = 0;
    m_srcRect.w = m_width;
    m_srcRect.h = m_height;
}

void VideoDisplayProcessor::Init()
{
    m_window = SDL_CreateWindow(
        "SDL Media Player",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        m_width,
        m_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!m_window)
    {
        LOG_EX_WARN_WITH_CONTEXT("SDL: could not create window");
        return;
    }
    SDL_GL_SetSwapInterval(1);
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
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
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
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
        m_texture,
        &m_srcRect,
        pkt->data,
        m_width,
        pkt->data + m_width * m_height,
        m_width / 2,
        pkt->data + m_width * m_height * 5 / 4,
        m_width / 2
    );

    ret = SDL_RenderClear(m_renderer);

    ret = SDL_RenderCopyEx(
        m_renderer,
        m_texture,
        nullptr,
        nullptr,
        0,
        nullptr,
        m_flip
    );

    SDL_RenderPresent(m_renderer);
}
