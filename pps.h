#ifndef SCSNIFF_PPS_H
#define SCSNIFF_PPS_H

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

int pps_analyze(struct pps *pps, unsigned char data);

int pps_done(struct pps *pps, unsigned *new_proto, unsigned *new_speed);

#endif // SCSNIFF_PPS_H
