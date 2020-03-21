#ifndef SCSNIFF_SESSION_H
#define SCSNIFF_SESSION_H

#include "atr.h"
#include "data.h"
#include "pps.h"

enum session_state {
    INIT = 0,
    ATR,
    IDLE,
    PPS,
    T0_DATA,
    T1_DATA,
};

typedef void (*set_baudrate_fn)(int fd, unsigned baudrate);

struct session {
    set_baudrate_fn set_baudrate;
    int serial_fd;
    unsigned base_baudrate;
    enum session_state state;
    struct atr atr;
    struct pps pps;
    struct data data;
    unsigned protocol_version;
};

void session_init(struct session *session, set_baudrate_fn set_baudrate,
                  int fd, unsigned baudrate);

void session_reset(struct session *session);

// Returns nonzero if byte was last in packet
int session_add_byte(struct session *session, unsigned char data);

#endif // SCSNIFF_SESSION_H
