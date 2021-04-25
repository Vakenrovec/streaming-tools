set (PROJECT_NAME network-lib)

set(HEADERS 
    Network.h
    NetPacket.h
    Agent.h
    Room.h
    StreamerSession.h
    ReceiverSession.h
)

set(SOURCES
    Agent.cpp
    Room.cpp
    StreamerSession.cpp
    ReceiverSession.cpp
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_library(${PROJECT_NAME} STATIC ${PROJECT_CONTENT})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
        ${Boost_INCLUDE_DIRS}
        ../utils-lib
        ../core-lib
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${Boost_LIBRARIES}
        utils-lib
        core-lib
)
