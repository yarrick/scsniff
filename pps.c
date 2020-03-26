#include "pps.h"

#include <string.h>

void pps_init(struct pps *pps) {
    memset(pps, 0, sizeof(struct pps));
    pps->proposal.pps1 = 0xff;
    pps->reply.pps1 = 0xff;
}

static int pps_msg_len(unsigned char pps0) {
    return 3 + __builtin_popcount(pps0 & 0xF0);
}

static int pps_parse(struct pps_msg *msg, unsigned char data, enum result packet_direction) {
    msg->bytes_seen++;
    if (msg->bytes_seen == 2) {
        // PPS0
        msg->msg_length = pps_msg_len(data);
        msg->pps0 = data;
    } else if (msg->bytes_seen == 3 && (msg->pps0 & 0x10)) {
        // PPS1
        msg->pps1 = data;
    }
    if (msg->bytes_seen == msg->msg_length) return packet_direction;
    return CONTINUE;
}

enum result pps_analyze(struct pps *pps, unsigned char data, unsigned *complete) {
    if (pps->proposal.bytes_seen < 2 || pps->proposal.bytes_seen < pps->proposal.msg_length) {
        return pps_parse(&pps->proposal, data, PACKET_TO_CARD);
    }
    if (pps->reply.bytes_seen < 2 || pps->reply.bytes_seen < pps->reply.msg_length) {
        int result = pps_parse(&pps->reply, data, PACKET_FROM_CARD);
        if (result == PACKET_FROM_CARD && complete) {
            *complete = 1;
        }
        return result;
    }
    // Out of bounds
    return STATE_ERROR;
}

void pps_result(struct pps *pps, unsigned *new_proto, unsigned *new_speed) {
    if (pps->proposal.bytes_seen == pps->proposal.msg_length &&
        pps->reply.bytes_seen == pps->reply.msg_length && pps->reply.msg_length > 0 &&
        (pps->proposal.pps0 & 0x0F) == (pps->reply.pps0 & 0x0F)) {
        // Both packets fully parsed and they agree on protocol version.

        if (new_proto) {
            *new_proto = pps->reply.pps0 & 0x0F;
        }
        if (new_speed && pps->proposal.pps1 == pps->reply.pps1 && pps->reply.pps1 != 0xFF) {
            *new_speed = pps->reply.pps1;
        }
    }
}
