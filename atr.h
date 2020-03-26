#ifndef SCSNIFF_ATR_H
#define SCSNIFF_ATR_H

#include "result.h"

enum atr_state {
    WAIT_T0 = 0,
    WAIT_TD,
    WAIT_END,
    ATR_DONE
};

struct atr {
    enum atr_state state;
    unsigned bytes_left;
    unsigned first_protocol_suggested;
    unsigned max_protocol_suggested;
    unsigned num_historical_bytes;
};

void atr_init(struct atr *atr);

enum result atr_analyze(struct atr *atr, unsigned char data, unsigned *complete);

void atr_result(struct atr *atr, unsigned *new_proto);

void atr_print_state(struct atr *atr);

#endif // SCSNIFF_ATR_H
