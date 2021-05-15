set(PROJECT_NAME credentials-lib)

set(HEADERS 
    credentials.h
)

set(SOURCES
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})

set(PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_library(${PROJECT_NAME} STATIC ${PROJECT_CONTENT})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
