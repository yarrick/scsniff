#ifndef SCSNIFF_ATR_H
#define SCSNIFF_ATR_H

enum atr_state {
    WAIT_T0 = 0,
    WAIT_TD,
    WAIT_END,
    ATR_DONE
};

struct atr_parser {
    enum atr_state state;
    unsigned bytes_left;
    unsigned max_protocol_suggested;
    unsigned num_historical_bytes;
};

void atr_init(struct atr_parser *parser);

int atr_at_end(struct atr_parser *parser, unsigned char data);

void atr_print_state(struct atr_parser *parser);

#endif // SCSNIFF_ATR_H
