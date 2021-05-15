set (PROJECT_NAME agent)

set(HEADERS 
    program.h
)

set(SOURCES
    program.cpp
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_executable(${PROJECT_NAME} ${PROJECT_CONTENT})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
        ../credentials-lib
        ${Boost_INCLUDE_DIRS}
        ../utils-lib
        ../network-lib
        ../core-lib
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        credentials-lib
        ${Boost_LIBRARIES}
        utils-lib
        network-lib
        core-lib
)
