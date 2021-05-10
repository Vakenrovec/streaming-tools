set (PROJECT_NAME tests)

set(HEADERS 
    catch.hpp
)

set(SOURCES
    main.cpp
    TestAudio.cpp
    TestVideo.cpp
    TestNetwork.cpp
    TestAudioVideo.cpp
    TestAll.cpp
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_executable(${PROJECT_NAME} ${PROJECT_CONTENT})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
        ${Boost_INCLUDE_DIRS}
        ${FFMPEG_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}
        ../utils-lib
        ../network-lib
        ../core-lib        
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${Boost_LIBRARIES}
        ${FFMPEG_LINK_LIBRARIES}
        ${SDL2_LINK_LIBRARIES}
        utils-lib
        network-lib
        core-lib        
)
