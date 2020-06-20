#ifndef SCSNIFF_ATR_H
#define SCSNIFF_ATR_H

#include "result.h"

#define ATR_TS_DIRECT (0x3B)
#define ATR_TS_INVERSE (0x3F)

enum atr_state {
    WAIT_T0 = 0,
    WAIT_TA_TD,
    WAIT_TD,
    WAIT_TA_END,
    WAIT_END,
    ATR_DONE
};

struct atr {
    enum atr_state state;
    unsigned bytes_left;
    unsigned first_protocol_suggested;
    unsigned latest_protocol;
    unsigned num_historical_bytes;
    unsigned t_cycle;
    unsigned ta1_value;
    unsigned ta2_seen;
};

void atr_init(struct atr *atr);

enum result atr_analyze(struct atr *atr, unsigned char data, unsigned *complete);

void atr_result(struct atr *atr, unsigned *new_proto, unsigned *new_speed);

#endif // SCSNIFF_ATR_H
