#ifndef SCSNIFF_SESSION_H
#define SCSNIFF_SESSION_H

#include "atr.h"

enum session_state {
    INIT = 0,
    ATR,
    IDLE,
    PPS,
    DATA
};

struct session {
    unsigned base_baudrate;
    enum session_state state;
    struct atr_parser atr;
};

typedef void (*set_baudrate_fn)(int fd, unsigned baudrate);

void session_init(struct session *session, unsigned baudrate, set_baudrate_fn set_baudrate);

// Returns nonzero if byte was last in packet
int session_add_byte(struct session *session, unsigned char data);

#endif // SCSNIFF_SESSION_H
