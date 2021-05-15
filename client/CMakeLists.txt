### Client 

set (PROJECT_NAME client)

set(HEADERS 
    program.h
    client.h
    streamer.h
    receiver.h
)

set(SOURCES
    program.cpp
    streamer.cpp
    receiver.cpp
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_executable(${PROJECT_NAME} ${PROJECT_CONTENT})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
        ../credentials-lib
        ../utils-lib
        ../core-lib
        ../network-lib
        ${Boost_INCLUDE_DIRS}
        ${FFMPEG_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        credentials-lib
        utils-lib
        core-lib
        network-lib
        ${Boost_LIBRARIES}
        ${FFMPEG_LINK_LIBRARIES}
        ${SDL2_LINK_LIBRARIES}
)

### GUI Client

set (PROJECT_NAME guiclient)

set(HEADERS 
    GUIProgram.h
    client.h
    streamer.h
    receiver.h
)

set(SOURCES
    GUIProgram.cpp
    streamer.cpp
    receiver.cpp
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_executable(${PROJECT_NAME} ${PROJECT_CONTENT})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
        ../utils-lib
        ../core-lib
        ../network-lib
        ${Boost_INCLUDE_DIRS}
        ${FFMPEG_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}
        ${GTKMM_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        utils-lib
        core-lib
        network-lib
        ${Boost_LIBRARIES}
        ${FFMPEG_LINK_LIBRARIES}
        ${SDL2_LINK_LIBRARIES}
        ${GTKMM_LINK_LIBRARIES}
)
