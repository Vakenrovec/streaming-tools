set (PROJECT_NAME tests)

set(HEADERS 
    catch.hpp
)

set(SOURCES
    main.cpp
    TestEncodeDecodeVP8.cpp
    TestAgent.cpp
    TestAudio.cpp
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_executable(${PROJECT_NAME} ${PROJECT_CONTENT})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
        ${Boost_INCLUDE_DIRS}
        ../utils-lib
        ../network-lib
        ../core-lib
        ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${Boost_LIBRARIES}
        utils-lib
        network-lib
        core-lib
        ${FFMPEG_LINK_LIBRARIES}
)
