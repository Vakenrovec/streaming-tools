# How to build on local machine
## Prerequisites

1. install cmake

        sudo apt-get install cmake

2. install libvpx

        sudo apt-get install libvpx-dev

3. install SDL2

        sudo apt-get install libsdl1.2-dev

4. install and build FFMPEG [here](https://trac.ffmpeg.org/wiki/CompilationGuide/Ubuntu) 

5. install and build boost [here](https://www.boost.org/doc/libs/1_75_0/more/getting_started/unix-variants.html)

## Build

    mkdir build
    cd build
    cmake ..
    make -j 4

# Some useful ffplay commands to open images

    ffplay -s 1280x720 -pixel_format yuv420p -f rawvideo -i ./webcam_output.yuv420p
