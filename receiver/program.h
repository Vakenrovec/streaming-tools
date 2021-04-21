#pragma once

#include "devices/WebCamera.h"
#include "codecs/vp8.h"
#include "Image.h"
#include "ImageUtils.h"
#include "Logger.h"
#include "rtp/RTPFragmenter.h"
#include "ReceiverSession.h"
#include "ThreadPoolManager.h"
#include "MediaPacket.h"
#include "rtp/RTPVp8Depay.h"
#include <fstream>
#include <cstdint>
#include <memory>
#include <list>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/avstring.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#ifdef __cplusplus
}
#endif

/**
 * Prevents SDL from overriding main().
 */
#ifdef __MINGW32__
#undef main
#endif
