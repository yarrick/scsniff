#include "session.h"

#include <string.h>

static void session_reset(struct session *session) {
    session->state = INIT;
    atr_init(&session->atr);
    pps_init(&session->pps);
    session->set_baudrate(session->serial_fd, session->base_baudrate);
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
                if (end_atr) session->state = IDLE;
                return end_atr;
            }
        case IDLE:
            if (data == 0xFF && pps_done(&session->pps, NULL, NULL) == 0) {
                // PPS start byte
                session->state = PPS;
                return pps_analyze(&session->pps, data);
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
                    }
                }
                return end_pps;
            }
    }
    return 0;
}
