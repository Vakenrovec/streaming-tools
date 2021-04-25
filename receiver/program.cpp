#include "program.h"
#include <boost/program_options.hpp>
#include <iostream>

using namespace boost::program_options;

int main(int argc, char* argv[]) {
    std::uint16_t udpPort = 35008;

    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Prints this help")
        ("local-udp-port", value<std::uint16_t>()->default_value(udpPort)->required(), "Local bind UDP port")
    ;

    try {
        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {  
            std::cout << desc << "\n";
            return 0;
        }

        if (vm.count("local-udp-port")) {
            udpPort = vm["local-udp-port"].as<std::uint16_t>();
        }
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    // SDL2    
    int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    if (ret != 0)
    {
        printf("Could not initialize SDL - %s\n.", SDL_GetError());
        return -1;
    }
    
    VP8Codec vp8codec;
    vp8codec.InitDecodeContext();

    std::uint16_t threadsCount = 1;
    ThreadPoolManager::GetInstance()->Start(threadsCount);
    auto session = std::make_shared<ReceiverSession>(ThreadPoolManager::GetInstance()->Get());
    session->SetFPS(30);
    session->SetServerTcpEndpoint("192.11.0.3", 35005);
    session->SetServerUdpEndpoint("192.11.0.3", 35006);
    session->SetLocalUdpEndpoint("192.11.0.3", udpPort);
    session->SetLocalUdpIp("192.11.0.3");
    session->SetLocalUdpPort(udpPort);
    std::uint32_t id = 777;
    session->ConnectToStream(id);


    SDL_Event event;
    for(;;)
    {
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
            {
                SDL_Quit();
                break;
            }
        }
    }

    return 0;
}

// /**
//  * This thread reads in packets from the video queue, packet_queue_get(), decodes
//  * the video packets into a frame, and then calls the queue_picture() function to
//  * put the processed frame into the picture queue.
//  *
//  * @param   arg global VideoState reference.
//  *
//  * @return
//  */
// int video_thread(void * arg)
// {
//     // retrieve global VideoState reference
//     VideoState * videoState = (VideoState *)arg;

//     AVPacket * packet = av_packet_alloc();
//     if (packet == NULL)
//     {
//         printf("Could not alloc packet.\n");
//         return -1;
//     }

//     int frameFinished;

//     // allocate a new AVFrame, used to decode video packets
//     static AVFrame * pFrame = NULL;
//     pFrame = av_frame_alloc();
//     if (!pFrame)
//     {
//         printf("Could not allocate AVFrame.\n");
//         return -1;
//     }

//     for (;;)
//     {
//         if (packet_queue_get(&videoState->videoq, packet, 1) < 0)
//         {
//             break;
//         }

//         // decode frame
//         frameFinished = 1;
        
//         // Did we get a video frame?
//         if (frameFinished)
//         {
//             if(queue_picture(videoState, pFrame) < 0)
//             {
//                 break;
//             }
//         }
//     }

//     return 0;
// }

// /**
//  * Allocates a new SDL_Overlay for the VideoPicture struct referenced by the
//  * global VideoState struct reference.
//  * The remaining VideoPicture struct fields are also updated.
//  *
//  * @param   userdata    global VideoState reference.
//  */
// void alloc_picture(void * userdata)
// {
//     // retrieve global VideoState reference.
//     VideoState * videoState = (VideoState *)userdata;

//     // retrieve the VideoPicture pointed by the queue write index
//     VideoPicture * videoPicture;
//     videoPicture = &videoState->pictq[videoState->pictq_windex];

//     // check if the SDL_Overlay is allocated
//     if (videoPicture->frame)
//     {
//         // we already have an AVFrame allocated, free memory
//         av_frame_free(&videoPicture->frame);
//         av_free(videoPicture->frame);
//     }

//     // lock global screen mutex
//     SDL_LockMutex(screen_mutex);

//     // get the size in bytes required to store an image with the given parameters
//     int numBytes;
//     numBytes = av_image_get_buffer_size(
//             AV_PIX_FMT_YUV420P,
//             1280,
//             720,
//             32
//     );

//     // allocate image data buffer
//     uint8_t * buffer = NULL;
//     buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

//     // alloc the AVFrame later used to contain the scaled frame
//     videoPicture->frame = av_frame_alloc();
//     if (videoPicture->frame == NULL)
//     {
//         printf("Could not allocate frame.\n");
//         return;
//     }

//     // The fields of the given image are filled in by using the buffer which points to the image data buffer.
//     av_image_fill_arrays(
//             videoPicture->frame->data,
//             videoPicture->frame->linesize,
//             buffer,
//             AV_PIX_FMT_YUV420P,
//             1280,
//             720,
//             32
//     );

//     // unlock global screen mutex
//     SDL_UnlockMutex(screen_mutex);

//     // update VideoPicture struct fields
//     videoPicture->width = 1280;
//     videoPicture->height = 720;
//     videoPicture->allocated = 1;
// }

// /**
//  * Waits for space in the VideoPicture queue. Allocates a new SDL_Overlay in case
//  * it is not already allocated or has a different width/height. Converts the given
//  * decoded AVFrame to an AVPicture using specs supported by SDL and writes it in the
//  * VideoPicture queue.
//  *
//  * @param   videoState  global VideoState reference.
//  * @param   pFrame      AVFrame to be inserted in the VideoState->pictq (as an AVPicture).
//  *
//  * @return              < 0 in case the global quit flag is set, 0 otherwise.
//  */
// int queue_picture(VideoState * videoState, AVFrame * pFrame)
// {
//     // lock VideoState->pictq mutex
//     SDL_LockMutex(videoState->pictq_mutex);

//     // wait until we have space for a new pic in VideoState->pictq
//     while (videoState->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE && !videoState->quit)
//     {
//         SDL_CondWait(videoState->pictq_cond, videoState->pictq_mutex);
//     }

//     // unlock VideoState->pictq mutex
//     SDL_UnlockMutex(videoState->pictq_mutex);

//     // check global quit flag
//     if (videoState->quit)
//     {
//         return -1;
//     }

//     // retrieve video picture using the queue write index
//     VideoPicture * videoPicture;
//     videoPicture = &videoState->pictq[videoState->pictq_windex];

//     // if the VideoPicture SDL_Overlay is not allocated or has a different width/height
//     if (!videoPicture->frame ||
//         videoPicture->width != 1280 ||
//         videoPicture->height != 720)
//     {
//         // set SDL_Overlay not allocated
//         videoPicture->allocated = 0;

//         // allocate a new SDL_Overlay for the VideoPicture struct
//         alloc_picture(videoState);

//         // check global quit flag
//         if(videoState->quit)
//         {
//             return -1;
//         }
//     }

//     // check the new SDL_Overlay was correctly allocated
//     if (videoPicture->frame)
//     {
//         // set VideoPicture AVFrame info using the last decoded frame
//         videoPicture->frame->pict_type = pFrame->pict_type;
//         videoPicture->frame->pts = pFrame->pts;
//         videoPicture->frame->pkt_dts = pFrame->pkt_dts;
//         videoPicture->frame->key_frame = pFrame->key_frame;
//         videoPicture->frame->coded_picture_number = pFrame->coded_picture_number;
//         videoPicture->frame->display_picture_number = pFrame->display_picture_number;
//         videoPicture->frame->width = pFrame->width;
//         videoPicture->frame->height = pFrame->height;

//         // scale the image in pFrame->data and put the resulting scaled image in pict->data
//         sws_scale(
//                 videoState->sws_ctx,
//                 (uint8_t const * const *)pFrame->data,
//                 pFrame->linesize,
//                 0,
//                 720,
//                 videoPicture->frame->data,
//                 videoPicture->frame->linesize
//         );

//         // update VideoPicture queue write index
//         ++videoState->pictq_windex;

//         // if the write index has reached the VideoPicture queue size
//         if(videoState->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE)
//         {
//             // set it to 0
//             videoState->pictq_windex = 0;
//         }

//         // lock VideoPicture queue
//         SDL_LockMutex(videoState->pictq_mutex);

//         // increase VideoPicture queue size
//         videoState->pictq_size++;

//         // unlock VideoPicture queue
//         SDL_UnlockMutex(videoState->pictq_mutex);
//     }

//     return 0;
// }


// /**
//  * Pulls from the VideoPicture queue when we have something, sets our timer for
//  * when the next video frame should be shown, calls the video_display() method to
//  * actually show the video on the screen, then decrements the counter on the queue,
//  * and decreases its size.
//  *
//  * @param   userdata    SDL_UserEvent->data1;   User defined data pointer.
//  */
// void video_refresh_timer(void * userdata)
// {
//     // retrieve global VideoState reference
//     VideoState * videoState = (VideoState *)userdata;

//     /* we will later see how to properly use this */
//     VideoPicture * videoPicture;

//     // check the VideoPicture queue contains decoded frames
//     if (videoState->pictq_size == 0)
//     {
//         schedule_refresh(videoState, 39);
//     }
//     else
//     {
//         // get VideoPicture reference using the queue read index
//         videoPicture = &videoState->pictq[videoState->pictq_rindex];

//         schedule_refresh(videoState, 39);

//         // show the frame on the SDL_Surface (the screen)
//         video_display(videoState);

//         // update read index for the next frame
//         if(++videoState->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE)
//         {
//             videoState->pictq_rindex = 0;
//         }

//         // lock VideoPicture queue mutex
//         SDL_LockMutex(videoState->pictq_mutex);

//         // decrease VideoPicture queue size
//         videoState->pictq_size--;

//         // notify other threads waiting for the VideoPicture queue
//         SDL_CondSignal(videoState->pictq_cond);

//         // unlock VideoPicture queue mutex
//         SDL_UnlockMutex(videoState->pictq_mutex);
//     }
// }

// /**
//  * Schedules video updates - every time we call this function, it will set the
//  * timer, which will trigger an event, which will have our main() function in turn
//  * call a function that pulls a frame from our picture queue and displays it.
//  *
//  * @param   videoState  global VideoState reference.
//  *
//  * @param   delay       the delay, expressed in milliseconds, before display the
//  *                      next video frame on the screen.
//  */
// static void schedule_refresh(VideoState * videoState, int delay)
// {
//     SDL_AddTimer(delay, sdl_refresh_timer_cb, videoState);
// }

// /**
//  * Pushes an SDL_Event of type FF_REFRESH_EVENT to the events queue.
//  *
//  * @param   interval
//  * @param   opaque
//  *
//  * @return
//  */
// static Uint32 sdl_refresh_timer_cb(Uint32 interval, void * opaque)
// {
//     // create SDL_Event of type FF_REFRESH_EVENT
//     SDL_Event event;
//     event.type = FF_REFRESH_EVENT;
//     event.user.data1 = opaque;

//     // push the event to the events queue
//     SDL_PushEvent(&event);

//     // return 0 to stop the timer
//     return 0;
// }

// /**
//  * Retrieves the video aspect ratio first, which is just the width divided by the
//  * height. Then it scales the movie to fit as big as possible in our screen
//  * (SDL_Surface). Then it centers the movie, and calls SDL_DisplayYUVOverlay()
//  * to update the surface, making sure we use the screen mutex to access it.
//  *
//  * @param   videoState  the global VideoState reference.
//  */
// void video_display(VideoState * videoState)
// {
//     // reference for the next VideoPicture to be displayed
//     VideoPicture * videoPicture;

//     float aspect_ratio;

//     int w, h, x, y;

//     // get next VideoPicture to be displayed from the VideoPicture queue
//     videoPicture = &videoState->pictq[videoState->pictq_rindex];

//     if (videoPicture->frame)
//     {
//         // if (videoState->video_ctx->sample_aspect_ratio.num == 0)
//         {
//             aspect_ratio = 0;
//         }
//         // else
//         // {
//         //     aspect_ratio = av_q2d(videoState->video_ctx->sample_aspect_ratio) * 1280 / 720;
//         // }

//         if (aspect_ratio <= 0.0)
//         {
//             aspect_ratio = (float)1280 /
//                            (float)720;
//         }

//         // get the size of a window's client area
//         int screen_width;
//         int screen_height;
//         SDL_GetWindowSize(screen, &screen_width, &screen_height);

//         // global SDL_Surface height
//         h = screen_height;

//         // retrieve width using the calculated aspect ratio and the screen height
//         w = ((int) rint(h * aspect_ratio)) & -3;

//         // if the new width is bigger than the screen width
//         if (w > screen_width)
//         {
//             // set the width to the screen width
//             w = screen_width;

//             // recalculate height using the calculated aspect ratio and the screen width
//             h = ((int) rint(w / aspect_ratio)) & -3;
//         }

//         x = (screen_width - w);
//         y = (screen_height - h);

//         // check the number of frames to decode was not exceeded
//         if (++videoState->currentFrameIndex < videoState->maxFramesToDecode)
//         {
//             // dump information about the frame being rendered
//             printf(
//                     "Frame %c (%d) pts %d dts %d key_frame %d [coded_picture_number %d, display_picture_number %d, %dx%d]\n",
//                     av_get_picture_type_char(videoPicture->frame->pict_type),
//                     1,
//                     videoPicture->frame->pts,
//                     videoPicture->frame->pkt_dts,
//                     videoPicture->frame->key_frame,
//                     videoPicture->frame->coded_picture_number,
//                     videoPicture->frame->display_picture_number,
//                     videoPicture->frame->width,
//                     videoPicture->frame->height
//             );

//             // set blit area x and y coordinates, width and height
//             SDL_Rect rect;
//             rect.x = x;
//             rect.y = y;
//             rect.w = 2*w;
//             rect.h = 2*h;

//             // lock screen mutex
//             SDL_LockMutex(screen_mutex);

//             // update the texture with the new pixel data
//             SDL_UpdateYUVTexture(
//                     videoState->texture,
//                     &rect,
//                     videoPicture->frame->data[0],
//                     videoPicture->frame->linesize[0],
//                     videoPicture->frame->data[1],
//                     videoPicture->frame->linesize[1],
//                     videoPicture->frame->data[2],
//                     videoPicture->frame->linesize[2]
//             );

//             // clear the current rendering target with the drawing color
//             SDL_RenderClear(videoState->renderer);

//             // copy a portion of the texture to the current rendering target
//             SDL_RenderCopy(videoState->renderer, videoState->texture, NULL, NULL);

//             // update the screen with any rendering performed since the previous call
//             SDL_RenderPresent(videoState->renderer);

//             // unlock screen mutex
//             SDL_UnlockMutex(screen_mutex);
//         }
//         else
//         {
//             // create an SDLEvent of type FF_QUIT_EVENT
//             SDL_Event event;
//             event.type = FF_QUIT_EVENT;
//             event.user.data1 = videoState;

//             // push the event
//             SDL_PushEvent(&event);
//         }
//     }
// }

















// /**
//  * Initialize the given PacketQueue.
//  *
//  * @param q the PacketQueue to be initialized.
//  */
// void packet_queue_init(PacketQueue * q)
// {
//     // alloc memory for the audio queue
//     memset(
//             q,
//             0,
//             sizeof(PacketQueue)
//     );

//     // Returns the initialized and unlocked mutex or NULL on failure
//     q->mutex = SDL_CreateMutex();
//     if (!q->mutex)
//     {
//         // could not create mutex
//         printf("SDL_CreateMutex Error: %s.\n", SDL_GetError());
//         return;
//     }

//     // Returns a new condition variable or NULL on failure
//     q->cond = SDL_CreateCond();
//     if (!q->cond)
//     {
//         // could not create condition variable
//         printf("SDL_CreateCond Error: %s.\n", SDL_GetError());
//         return;
//     }
// }

// /**
//  * Put the given AVPacket in the given PacketQueue.
//  *
//  * @param  queue    the queue to be used for the insert
//  * @param  packet   the AVPacket to be inserted in the queue
//  *
//  * @return          0 if the AVPacket is correctly inserted in the given PacketQueue.
//  */
// int packet_queue_put(PacketQueue * queue, AVPacket * packet)
// {
//     // alloc the new AVPacketList to be inserted in the audio PacketQueue
//     AVPacketList * avPacketList;
//     avPacketList = (AVPacketList*)av_malloc(sizeof(AVPacketList));

//     // check the AVPacketList was allocated
//     if (!avPacketList)
//     {
//         return -1;
//     }

//     // add reference to the given AVPacket
//     avPacketList->pkt = * packet;

//     // the new AVPacketList will be inserted at the end of the queue
//     avPacketList->next = NULL;

//     // lock mutex
//     SDL_LockMutex(queue->mutex);

//     // check the queue is empty
//     if (!queue->last_pkt)
//     {
//         // if it is, insert as first
//         queue->first_pkt = avPacketList;
//     }
//     else
//     {
//         // if not, insert as last
//         queue->last_pkt->next = avPacketList;
//     }

//     // point the last AVPacketList in the queue to the newly created AVPacketList
//     queue->last_pkt = avPacketList;

//     // increase by 1 the number of AVPackets in the queue
//     queue->nb_packets++;

//     // increase queue size by adding the size of the newly inserted AVPacket
//     queue->size += avPacketList->pkt.size;

//     // notify packet_queue_get which is waiting that a new packet is available
//     SDL_CondSignal(queue->cond);

//     // unlock mutex
//     SDL_UnlockMutex(queue->mutex);

//     return 0;
// }

// /**
//  * Get the first AVPacket from the given PacketQueue.
//  *
//  * @param  queue      The PacketQueue to extract from
//  * @param  packet    The first AVPacket extracted from the queue
//  * @param  blocking  0 to avoid waiting for an AVPacket to be inserted in the given
//  *                queue, != 0 otherwise.
//  *
//  * @return        < 0 if returning because the quit flag is set, 0 if the queue
//  *                is empty, 1 if it is not empty and a packet was extract (pkt)
//  */
// static int packet_queue_get(PacketQueue * queue, AVPacket * packet, int blocking)
// {
//     int ret;

//     AVPacketList * avPacketList;

//     // lock mutex
//     SDL_LockMutex(queue->mutex);

//     for (;;)
//     {
//         // check quit flag
//         if (global_video_state->quit)
//         {
//             ret = -1;
//             break;
//         }

//         // point to the first AVPacketList in the queue
//         avPacketList = queue->first_pkt;

//         // if the first packet is not NULL, the queue is not empty
//         if (avPacketList)
//         {
//             // place the second packet in the queue at first position
//             queue->first_pkt = avPacketList->next;

//             // check if queue is empty after removal
//             if (!queue->first_pkt)
//             {
//                 // first_pkt = last_pkt = NULL = empty queue
//                 queue->last_pkt = NULL;
//             }

//             // decrease the number of packets in the queue
//             queue->nb_packets--;

//             // decrease the size of the packets in the queue
//             queue->size -= avPacketList->pkt.size;

//             // point packet to the extracted packet, this will return to the calling function
//             *packet = avPacketList->pkt;

//             // free memory
//             av_free(avPacketList);

//             ret = 1;
//             break;
//         }
//         else if (!blocking)
//         {
//             ret = 0;
//             break;
//         }
//         else
//         {
//             // unlock mutex and wait for cond signal, then lock mutex again
//             SDL_CondWait(queue->cond, queue->mutex);
//         }
//     }

//     // unlock mutex
//     SDL_UnlockMutex(queue->mutex);

//     return ret;
// }


// /**
//  * Pull in data from audio_decode_frame(), store the result in an intermediary
//  * buffer, attempt to write as many bytes as the amount defined by len to
//  * stream, and get more data if we don't have enough yet, or save it for later
//  * if we have some left over.
//  *
//  * @param userdata  the pointer we gave to SDL.
//  * @param stream    the buffer we will be writing audio data to.
//  * @param len       the size of that buffer.
//  */
// void audio_callback(void * userdata, Uint8 * stream, int len)
// {
//     // retrieve the VideoState
//     VideoState * videoState = (VideoState *)userdata;

//     int len1 = -1;
//     unsigned int audio_size = -1;

//     while (len > 0)
//     {
//         // check global quit flag
//         if (global_video_state->quit)
//         {
//             return;
//         }

//         if (videoState->audio_buf_index >= videoState->audio_buf_size)
//         {
//             // we have already sent all avaialble data; get more
//             audio_size = audio_decode_frame(videoState, videoState->audio_buf, sizeof(videoState->audio_buf));

//             // if error
//             if (audio_size < 0)
//             {
//                 // output silence
//                 videoState->audio_buf_size = 1024;

//                 // clear memory
//                 memset(videoState->audio_buf, 0, videoState->audio_buf_size);
//                 printf("audio_decode_frame() failed.\n");
//             }
//             else
//             {
//                 videoState->audio_buf_size = audio_size;
//             }

//             videoState->audio_buf_index = 0;
//         }

//         len1 = videoState->audio_buf_size - videoState->audio_buf_index;

//         if (len1 > len)
//         {
//             len1 = len;
//         }

//         // copy data from audio buffer to the SDL stream
//         memcpy(stream, (uint8_t *)videoState->audio_buf + videoState->audio_buf_index, len1);

//         len -= len1;
//         stream += len1;
//         videoState->audio_buf_index += len1;
//     }
// }

// /**
//  * Get a packet from the queue if available. Decode the extracted packet. Once
//  * we have the frame, resample it and simply copy it to our audio buffer, making
//  * sure the data_size is smaller than our audio buffer.
//  *
//  * @param  aCodecCtx  the audio AVCodecContext used for decoding
//  * @param  audio_buf  the audio buffer to write into
//  * @param  buf_size   the size of the audio buffer, 1.5 larger than the one
//  *                    provided by FFmpeg
//  *
//  * @return            0 if everything goes well, -1 in case of error or quit
//  */
// int audio_decode_frame(VideoState * videoState, uint8_t * audio_buf, int buf_size)
// {
//     // AVPacket * avPacket = av_packet_alloc();
//     // static uint8_t * audio_pkt_data = NULL;
//     // static int audio_pkt_size = 0;

//     // // allocate a new frame, used to decode audio packets
//     // static AVFrame * avFrame = NULL;
//     // avFrame = av_frame_alloc();
//     // if (!avFrame)
//     // {
//     //     printf("Could not allocate AVFrame.\n");
//     //     return -1;
//     // }

//     // int len1 = 0;
//     // int data_size = 0;

//     // for (;;)
//     // {
//     //     // check global quit flag
//     //     if (videoState->quit)
//     //     {
//     //         return -1;
//     //     }

//     //     while (audio_pkt_size > 0)
//     //     {
//     //         int got_frame = 0;

//     //         int ret = avcodec_receive_frame(videoState->audio_ctx, avFrame);
//     //         if (ret == 0)
//     //         {
//     //             got_frame = 1;
//     //         }
//     //         if (ret == AVERROR(EAGAIN))
//     //         {
//     //             ret = 0;
//     //         }
//     //         if (ret == 0)
//     //         {
//     //             ret = avcodec_send_packet(videoState->audio_ctx, avPacket);
//     //         }
//     //         if (ret == AVERROR(EAGAIN))
//     //         {
//     //             ret = 0;
//     //         }
//     //         else if (ret < 0)
//     //         {
//     //             printf("avcodec_receive_frame error");
//     //             return -1;
//     //         }
//     //         else
//     //         {
//     //             len1 = avPacket->size;
//     //         }

//     //         if (len1 < 0)
//     //         {
//     //             // if error, skip frame
//     //             audio_pkt_size = 0;
//     //             break;
//     //         }

//     //         audio_pkt_data += len1;
//     //         audio_pkt_size -= len1;
//     //         data_size = 0;

//     //         if (got_frame)
//     //         {
//     //             // audio resampling
//     //             data_size = audio_resampling(
//     //                     videoState->audio_ctx,
//     //                     avFrame,
//     //                     AV_SAMPLE_FMT_S16,
//     //                     videoState->audio_ctx->channels,
//     //                     videoState->audio_ctx->sample_rate,
//     //                     audio_buf
//     //             );

//     //             assert(data_size <= buf_size);
//     //         }

//     //         if (data_size <= 0)
//     //         {
//     //             // no data yet, get more frames
//     //             continue;
//     //         }

//     //         // we have the data, return it and come back for more later
//     //         return data_size;
//     //     }

//     //     if (avPacket->data)
//     //     {
//     //         // wipe the packet
//     //         av_packet_unref(avPacket);
//     //     }

//     //     // get more audio AVPacket
//     //     int ret = packet_queue_get(&videoState->audioq, avPacket, 1);

//     //     // if packet_queue_get returns < 0, the global quit flag was set
//     //     if (ret < 0)
//     //     {
//     //         return -1;
//     //     }

//     //     audio_pkt_data = avPacket->data;
//     //     audio_pkt_size = avPacket->size;
//     // }

//     return 0;
// }
