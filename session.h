#ifndef SCSNIFF_SESSION_H
#define SCSNIFF_SESSION_H

#include <sys/time.h>

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
    struct timeval time;
};

typedef void (*set_baudrate_fn)(int fd, unsigned baudrate);

typedef void (*completed_packet_fn)(struct packet *packet);

typedef void (*log_msg_fn)(const char *msg);

// Per-session fields, to be reset when card is.
struct current_session {
    unsigned char buf[SESSION_BUFLEN];
    unsigned buf_index;
    enum session_state state;
    struct timeval buf_time;
    unsigned protocol_version;
    unsigned baudrate;
    struct atr atr;
    struct pps pps;
    struct data data;
};

struct session {
    set_baudrate_fn set_baudrate;
    int serial_fd;
    unsigned base_baudrate;
    completed_packet_fn completed_packet;
    log_msg_fn log_msg;
    struct current_session curr;
};

void session_init(struct session *session, completed_packet_fn completed_packet,
                  set_baudrate_fn set_baudrate, log_msg_fn log_msg, int fd,
                  unsigned baudrate);

void session_reset(struct session *session);

void session_add_byte(struct session *session, unsigned char data);

#endif // SCSNIFF_SESSION_H
