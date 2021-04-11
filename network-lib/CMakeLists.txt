set (PROJECT_NAME network-lib)

set(HEADERS 
    udp.h
)

set(SOURCES
    udp.cpp
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_library(${PROJECT_NAME} STATIC ${PROJECT_CONTENT})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        # ${FFMPEG_LIBRARIES}
)
