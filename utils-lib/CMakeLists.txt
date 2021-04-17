set (PROJECT_NAME utils-lib)

set(HEADERS 
    FileUtils.h
    ImageUtils.h
    Logger.h
    DateTimeUtils.h
    ThreadPoolManager.h
)

set(SOURCES
    FileUtils.cpp
    ImageUtils.cpp
    Logger.cpp
    DateTimeUtils.cpp
    ThreadPoolManager.cpp
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_library(${PROJECT_NAME} STATIC ${PROJECT_CONTENT})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
        ${Boost_INCLUDE_DIRS}
        ../core-lib
        ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${FFMPEG_LINK_LIBRARIES}
)
