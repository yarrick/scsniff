#include "atr.h"

#include <stdio.h>
#include <string.h>


void atr_init(struct atr_parser *parser) {
    memset(parser, 0, sizeof(struct atr_parser));
    // State WAIT_T0, need two bytes
    parser->bytes_left = 2;
}

static int handle_t_bits(struct atr_parser *parser, unsigned char data) {
    unsigned t_bytes = __builtin_popcount(data & 0xF0);
    if (data & 0x80) {
        // Another TD byte coming
        parser->bytes_left = t_bytes;
        parser->state = WAIT_TD;
    } else {
        // T bytes and historical data left
        parser->bytes_left = t_bytes + parser->num_historical_bytes;
        // Add TCK byte if higher protocol suggested
        if (parser->max_protocol_suggested > 0) parser->bytes_left++;
        parser->state = WAIT_END;
    }
    if (parser->bytes_left == 0) {
        // No T bytes or historical bytes
        parser->state = ATR_DONE;
        return 1;
    }
    return 0;
}

int atr_at_end(struct atr_parser *parser, unsigned char data) {
    // Already done?
    if (parser->state == ATR_DONE) return 1;
    parser->bytes_left--;
    // Waiting for more data?
    if (parser->bytes_left > 0) {
        return 0;
    }
    switch (parser->state) {
        case WAIT_T0:
            // Lower nibble of T0 is number of historical bytes
            parser->num_historical_bytes = data & 0xF;
            return handle_t_bits(parser, data);
        case WAIT_TD:
            // Lower nibble of TD is suggested protocol
            if (data & 0xF > parser->max_protocol_suggested) {
                parser->max_protocol_suggested = data & 0xF;
            }
            return handle_t_bits(parser, data);
        case WAIT_END:
            parser->state = ATR_DONE;
            return 1;
    }
}

void atr_print_state(struct atr_parser *parser) {
    static const char* state_names[] =
        { "WAIT_T0", "WAIT_TD", "WAIT_END", "ATR_DONE" };
    printf("State %s, need %d bytes (%d hist bytes, max proto %d)\n",
        state_names[parser->state], parser->bytes_left, parser->num_historical_bytes,
        parser->max_protocol_suggested);
}
