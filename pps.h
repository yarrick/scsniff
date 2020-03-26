#ifndef SCSNIFF_PPS_H
#define SCSNIFF_PPS_H

#include "result.h"

struct pps_msg {
    unsigned bytes_seen;
    unsigned msg_length;
    unsigned pps0;
    unsigned pps1;
};

struct pps {
    struct pps_msg proposal;
    struct pps_msg reply;
};

void pps_init(struct pps *pps);

enum result pps_analyze(struct pps *pps, unsigned char data, unsigned *complete);

void pps_result(struct pps *pps, unsigned *new_proto, unsigned *new_speed);

#endif // SCSNIFF_PPS_H
