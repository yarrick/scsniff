#ifndef SCSNIFF_SESSION_H
#define SCSNIFF_SESSION_H

#include "result.h"
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

#define SESSION_BUFLEN (512)

struct packet {
    unsigned char *data;
    unsigned data_length;
    enum result result;
};

typedef void (*set_baudrate_fn)(int fd, unsigned baudrate);

typedef void (*completed_packet_fn)(struct packet *packet);

struct session {
    unsigned char buf[SESSION_BUFLEN];
    unsigned buf_index;
    set_baudrate_fn set_baudrate;
    int serial_fd;
    unsigned base_baudrate;
    completed_packet_fn completed_packet;
    enum session_state state;
    struct atr atr;
    struct pps pps;
    struct data data;
    unsigned protocol_version;
};

void session_init(struct session *session, completed_packet_fn completed_packet,
                  set_baudrate_fn set_baudrate, int fd, unsigned baudrate);

void session_reset(struct session *session);

void session_add_byte(struct session *session, unsigned char data);

#endif // SCSNIFF_SESSION_H
