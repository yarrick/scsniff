#ifndef SCSNIFF_PACKET_H
#define SCSNIFF_PACKET_H

#include <sys/time.h>

enum result {
    CONTINUE = 0,      // Need more bytes
    PACKET_TO_CARD,    // End of packet, to card
    PACKET_FROM_CARD,  // End of packet, from card
    PACKET_UNKNOWN,    // End of packet, unclear direction
    STATE_ERROR = 99,  // Should not get more bytes
    NOISE = 100,       // Suspected noise, ignored
};

struct packet {
    unsigned char *data;
    unsigned data_length;
    enum result result;
    struct timeval time;
};

#endif // SCSNIFF_PACKET_H
