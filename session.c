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

static void send_packet(struct session *session, enum result result) {
    struct current_session *curr = &session->curr;
    struct packet packet;
    packet.data = curr->buf;
    packet.data_length = curr->buf_index;
    packet.result = result;
    packet.time = curr->buf_time;
    session->completed_packet(&packet);
}

void session_reset(struct session *session) {
    struct current_session *curr = &session->curr;
    if (curr->buf_index > 0) {
        // Incomplete packet in buffer, consider it noise
        send_packet(session, NOISE);
        curr->buf_index = 0;
    }
    memset(&session->curr, 0, sizeof(struct current_session));
    atr_init(&curr->atr);
    pps_init(&curr->pps);
    data_init(&curr->data);
    session->set_baudrate(session->serial_fd, session->base_baudrate);
}

static void update_speed(struct session *session, unsigned speed, char *phase) {
    unsigned new_etu = clock_conversion(speed) / baud_divisor(speed);
    unsigned baudrate = session->base_baudrate * BASE_ETU / ((float) new_etu);
    fprintf(stderr, "== Switching to %d ticks per ETU (%d baud) after %s\n",
            new_etu, baudrate, phase);
    session->set_baudrate(session->serial_fd, baudrate);
}

void session_init(struct session *session, completed_packet_fn completed_packet,
                  set_baudrate_fn set_baudrate, int fd, unsigned baudrate) {
    memset(session, 0, sizeof(struct session));
    session->set_baudrate = set_baudrate;
    session->serial_fd = fd;
    session->base_baudrate = baudrate;
    session->completed_packet = completed_packet;
    session_reset(session);
}

static enum result analyze_byte(struct current_session *curr,
                                unsigned char data, unsigned *complete) {
    switch (curr->state) {
        case INIT:
            // Ignore early noise
            if (data == 0x00 || data == 0xFF) return NOISE;
            curr->state = ATR;
            // Fallthrough
        case ATR:
            return atr_analyze(&curr->atr, data, complete);
        case IDLE:
            if (data == 0xFF) {
                // PPS start byte
                curr->state = PPS;
                return pps_analyze(&curr->pps, data, complete);
            }
            if (curr->protocol_version == 0) {
                curr->state = T0_DATA;
                return data_t0_analyze(&curr->data, data);
            }
            if (curr->protocol_version == 1) {
                curr->state = T1_DATA;
                return data_t1_analyze(&curr->data, data);
            }
            return STATE_ERROR;
        case PPS:
            return pps_analyze(&curr->pps, data, complete);
        case T0_DATA:
            {
                int end_data = data_t0_analyze(&curr->data, data);
                if (end_data) curr->state = IDLE;
                return end_data;
            }
        case T1_DATA:
            {
                int end_data = data_t1_analyze(&curr->data, data);
                if (end_data) curr->state = IDLE;
                return end_data;
            }
    }
    return STATE_ERROR;
}

void session_add_byte(struct session *session, unsigned char data) {
    struct current_session *curr = &session->curr;
    enum result res = STATE_ERROR;
    unsigned phase_complete = 0;
    if (curr->buf_index < SESSION_BUFLEN) {
        curr->buf[curr->buf_index++] = data;
        res = analyze_byte(curr, data, &phase_complete);
    }
    if (curr->buf_index == 1) {
        // Record time of first byte
        gettimeofday(&curr->buf_time, NULL);
    }
    if (res) {
        send_packet(session, res);
        memset(curr->buf, 0, SESSION_BUFLEN);
        curr->buf_index = 0;
        if (phase_complete) {
            unsigned proto = 0xFF;
            unsigned speed = 0xFF;
            char *phase = "?";
            if (curr->state == ATR) {
                atr_result(&curr->atr, &proto);
                phase = "ATR";
            } else if (curr->state == PPS) {
                pps_result(&curr->pps, &proto, &speed);
                phase = "PPS";
            }
            if (proto != 0xFF && proto != curr->protocol_version) {
                curr->protocol_version = proto;
                fprintf(stderr, "== Switching to protocol T=%d after %s\n",
                        curr->protocol_version, phase);
            }
            if (speed != 0xFF) {
                update_speed(session, speed, phase);
            }
            curr->state = IDLE;
        }
    }
}
