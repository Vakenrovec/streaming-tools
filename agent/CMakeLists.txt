set (PROJECT_NAME agent)

set(HEADERS 
    agent.h
)

set(SOURCES
    agent.cpp
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
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${Boost_LIBRARIES}
        utils-lib
        network-lib
)
