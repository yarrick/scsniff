#include "session.h"

#include <stdio.h>
#include <string.h>

// From ISO/IEC 7816-3:2006 Section 8.3 Table 7
static unsigned clock_conversion(unsigned char speed) {
    switch (speed >> 4) {
        case 2: return 558;
        case 3: return 744;
        case 4: return 1116;
        case 5: return 1488;
        case 6: return 1860;
        case 9: return 512;
        case 0xA: return 768;
        case 0xB: return 1024;
        case 0xC: return 1536;
        case 0xD: return 2048;
        default: return 372;
    }
}

// From ISO/IEC 7816-3:2006 Section 8.3 Table 8
static unsigned baud_divisor(unsigned char speed) {
    switch (speed & 0x0F) {
        case 2: return 2;
        case 3: return 4;
        case 4: return 8;
        case 5: return 16;
        case 6: return 32;
        case 7: return 64;
        case 8: return 12;
        case 9: return 20;
        default: return 1;
    }
}

#define BASE_ETU (372)

void session_reset(struct session *session) {
    session->state = INIT;
    atr_init(&session->atr);
    pps_init(&session->pps);
    data_init(&session->data);
    session->set_baudrate(session->serial_fd, session->base_baudrate);
}

static void update_speed(struct session *session, unsigned speed) {
    unsigned new_etu = clock_conversion(speed) / baud_divisor(speed);
    unsigned baudrate = session->base_baudrate * BASE_ETU / ((float) new_etu);
    fprintf(stderr,"\n== Switching session to %d ETU => %d baud\n", new_etu, baudrate);
    session->set_baudrate(session->serial_fd, baudrate);
}

void session_init(struct session *session, set_baudrate_fn set_baudrate,
                  int fd, unsigned baudrate) {
    memset(session, 0, sizeof(struct session));
    session->set_baudrate = set_baudrate;
    session->serial_fd = fd;
    session->base_baudrate = baudrate;
    session_reset(session);
}

int session_add_byte(struct session *session, unsigned char data) {
    switch (session->state) {
        case INIT:
            // Ignore early noise
            if (data == 0x00 || data == 0xFF) return 1;
            session->state = ATR;
            // Fallthrough
        case ATR:
            {
                int end_atr = atr_analyze(&session->atr, data);
                if (end_atr) {
                    unsigned proto = 0xFF;
                    atr_done(&session->atr, &proto);
                    if (proto != 0xFF && proto != session->protocol_version) {
                        session->protocol_version = proto;
                        fprintf(stderr, "\n== Card requested protocol T=%d in ATR",
                                session->protocol_version);
                    }
                    session->state = IDLE;
                }
                return end_atr;
            }
        case IDLE:
            if (data == 0xFF && pps_done(&session->pps, NULL, NULL) == 0) {
                // PPS start byte
                session->state = PPS;
                return pps_analyze(&session->pps, data);
            }
            if (session->protocol_version == 0) {
                session->state = T0_DATA;
                return data_t0_analyze(&session->data, data);
            }
            if (session->protocol_version == 1) {
                session->state = T1_DATA;
                return data_t1_analyze(&session->data, data);
            }
            return 0;
        case PPS:
            {
                int end_pps = pps_analyze(&session->pps, data);
                if (end_pps) {
                    unsigned proto = 0;
                    unsigned speed = 0xFF;
                    if (pps_done(&session->pps, &proto, &speed)) {
                        session->state = IDLE;
                        session->protocol_version = proto;
                        fprintf(stderr, "\n== PPS completed, now using protocol T=%d",
                                session->protocol_version);
                        if (speed != 0xFF) {
                            update_speed(session, speed);
                        }
                    }
                }
                return end_pps;
            }
        case T0_DATA:
            {
                int end_data = data_t0_analyze(&session->data, data);
                if (end_data) session->state = IDLE;
                return end_data;
            }
        case T1_DATA:
            {
                int end_data = data_t1_analyze(&session->data, data);
                if (end_data) session->state = IDLE;
                return end_data;
            }
    }
    return 0;
}
