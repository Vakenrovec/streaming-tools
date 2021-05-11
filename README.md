# How to build on local machine
## Prerequisites

1. install cmake

        sudo apt-get install cmake

2. install libvpx

        sudo apt-get install libvpx-dev

3. what you need to do to install SDL2 is:

        install sdl2
        sudo apt install libsdl2-dev libsdl2-2.0-0 -y
        
        install sdl image  - if you want to display images
        sudo apt install libjpeg-dev libwebp-dev libtiff5-dev libsdl2-image-dev libsdl2-image-2.0-0 -y

        install sdl mixer - if you want sound
        sudo apt install libmikmod-dev libfishsound1-dev libsmpeg-dev liboggz2-dev libflac-dev libfluidsynth-dev libsdl2-mixer-dev libsdl2-mixer-2.0-0 -y

        install sdl true type fonts - if you want to use text
        sudo apt install libfreetype6-dev libsdl2-ttf-dev libsdl2-ttf-2.0-0 -y

        use
        `sdl2-config --cflags --libs` -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf    

4. install and build FFMPEG [here](https://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu) 

5. install and build boost [here](https://www.boost.org/doc/libs/1_75_0/more/getting_started/unix-variants.html)

## Build

    mkdir build
    cd build
    cmake ..
    make -j `nproc`

# Some useful ffplay commands to open images

    ffplay -s 1280x720 -pixel_format yuv420p -f rawvideo -i ./webcam_output.yv12

# Program default values

    server tcp ip = 192.11.0.3
    server tcp port = 35005

    server udp ip = 192.11.0.3
    server udp port = 35006

    streamer udp ip = 192.11.0.3
    streamer udp port = 35007

    receiver udp ip = 192.11.0.3
    receiver udp port = 35008

    streamId = 777
    width = 1280, height = 720
    bitrate = 4'000'000
    gopSize = 10

    save raw stream = false;
    raw stream dir = "/tmp/streams";
    raw stream filename = "stream.raw"

    disable video = false
    disable audio = false

# Command line arguments
## Server

    ./agent 

## Client

    ./client --streamer --stream-id 777
    ./client --receiver --stream-id 777
    ./client --receiver --stream-id 777 --receiver-udp-port 35009

## MediaPlayer

    ./mediaplayer --raw-stream-dir "/tmp/streams" --raw-stream-filename "stream.raw"

