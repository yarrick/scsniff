#ifndef SCSNIFF_PACKET_H
#define SCSNIFF_PACKET_H

#include <sys/time.h>

enum packet_dir {
    DIR_UNKNOWN        = 0x00,
    DIR_FROM_CARD      = 0x01,
    DIR_TO_CARD        = 0x02,
    INVALID_ERROR      = 0x0E,
    INVALID_NOISE      = 0x0F,
};

#define PACKET_DIR(res) ((res) & 0x0F)

enum result {
    CONTINUE            = 0x00,  // Need more bytes
    // Packet direction encoded in lower nibble.
    ANSWER_TO_RESET     = 0x100 | DIR_FROM_CARD,
    PPS_REQ             = 0x200 | DIR_TO_CARD,
    PPS_RESP            = 0x210 | DIR_FROM_CARD,
    T0_DATA_CMD_HEADER  = 0x300 | DIR_TO_CARD,
    T0_DATA_ACK         = 0x310 | DIR_FROM_CARD,
    T0_DATA_NULL        = 0x320 | DIR_FROM_CARD,
    T0_DATA_CMD_BODY    = 0x330 | DIR_TO_CARD,
    T0_DATA_RESP_BODY   = 0x340 | DIR_FROM_CARD,
    T0_DATA_UNK_BODY    = 0x350 | DIR_UNKNOWN,
    T0_DATA_RESP_SW     = 0x360 | DIR_FROM_CARD,
    T1_DATA_CMD         = 0x400 | DIR_TO_CARD,
    T1_DATA_RESP        = 0x410 | DIR_FROM_CARD,
    STATE_ERROR         = 0x1FF0 | INVALID_ERROR,  // Should not get more bytes
    NOISE               = 0x2FF0 | INVALID_NOISE,  // Suspected noise, ignored
};

struct packet {
    unsigned char *data;
    unsigned data_length;
    enum result result;
    struct timeval time;
};

#endif // SCSNIFF_PACKET_H
