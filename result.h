#ifndef SCSNIFF_RESULT_H
#define SCSNIFF_RESULT_H

enum result {
    CONTINUE = 0,      // Need more bytes
    PACKET_TO_CARD,    // End of packet, to card
    PACKET_FROM_CARD,  // End of packet, from card
    PACKET_UNKNOWN,    // End of packet, unclear direction
    STATE_ERROR = 99,  // Should not get more bytes
    NOISE = 100,       // Suspected noise, ignored
};

#endif // SCSNIFF_RESULT_H
