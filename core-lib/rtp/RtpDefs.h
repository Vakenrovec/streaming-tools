#pragma once

#include <cstdint>

#pragma pack(push,1)

typedef struct rtp_header {
    //first byte
    #if G_BYTE_ORDER == G_LITTLE_ENDIAN
        std::uint8_t          csrcCount:4;      /* CC field (extensions count) */
        std::uint8_t          extension:1;      /* X field */
        std::uint8_t          padding:1;        /* padding flag */
        std::uint8_t          version:2;        /* protocol version */
    #elif G_BYTE_ORDER == G_BIG_ENDIAN
        std::uint8_t          version:2;   
        std::uint8_t          padding:1;         
        std::uint8_t          extension:1;         
        std::uint8_t          csrcCount:4;        
    #else
        #error "G_BYTE_ORDER should be big or little endian."
    #endif
    //second byte
    #if G_BYTE_ORDER == G_LITTLE_ENDIAN
        std::uint8_t          pt:7;             /* PT field (payload type) */ // media type
        std::uint8_t          m:1;              /* M field (marker) */ // is packet last
    #elif G_BYTE_ORDER == G_BIG_ENDIAN
        std::uint8_t          m:1;              /* M field */
        std::uint8_t          pt:7;             /* PT field */
    #else
        #error "G_BYTE_ORDER should be big or little endian."
    #endif
        std::uint16_t              seq;         /* length of the recovery */
        std::uint64_t              ts;          /* Timestamp */
        std::uint32_t              ssrc;        /* synchronisation source */
} rtp_header_t;

typedef struct rtp_header_extension {
	std::uint16_t type;
	std::uint16_t length;
} rtp_header_extension_t;

typedef struct rtp_descriptor {
    #if G_BYTE_ORDER == G_LITTLE_ENDIAN
        std::uint8_t          PID:3;
        std::uint8_t          reserved1:1;
        std::uint8_t          s:1;              // is packet first
        std::uint8_t          n:1;  
        std::uint8_t          reserved2:1;
        std::uint8_t          x:1;
    #elif G_BYTE_ORDER == G_BIG_ENDIAN
        std::uint8_t          x:1;
        std::uint8_t          reserved2:1;
        std::uint8_t          n:1;
        std::uint8_t          s:1;  
        std::uint8_t          reserved1:1;
        std::uint8_t          PID:3;
    #else
        #error "G_BYTE_ORDER should be big or little endian."
    #endif
} rtp_descriptor_t;

#pragma pack(pop)
