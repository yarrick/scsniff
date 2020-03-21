#ifndef SCSNIFF_ATR_H
#define SCSNIFF_ATR_H

enum atr_state {
    WAIT_T0 = 0,
    WAIT_TD,
    WAIT_END,
    ATR_DONE
};

struct atr {
    enum atr_state state;
    unsigned bytes_left;
    unsigned max_protocol_suggested;
    unsigned num_historical_bytes;
};

void atr_init(struct atr *atr);

int atr_analyze(struct atr *atr, unsigned char data);

void atr_print_state(struct atr *atr);

#endif // SCSNIFF_ATR_H
