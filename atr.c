#include "atr.h"

#include <stdio.h>
#include <string.h>


void atr_init(struct atr *atr) {
    memset(atr, 0, sizeof(struct atr));
    // State WAIT_T0, need two bytes
    atr->bytes_left = 2;
    atr->first_protocol_suggested = 0xFF;
}

static int handle_t_bits(struct atr *atr, unsigned char data) {
    unsigned t_bytes = __builtin_popcount(data & 0xF0);
    if (data & 0x80) {
        // Another TD byte coming
        atr->bytes_left = t_bytes;
        atr->state = WAIT_TD;
    } else {
        // T bytes and historical data left
        atr->bytes_left = t_bytes + atr->num_historical_bytes;
        // Add TCK byte if higher protocol suggested
        if (atr->max_protocol_suggested > 0) atr->bytes_left++;
        atr->state = WAIT_END;
    }
    if (atr->bytes_left == 0) {
        // No T bytes or historical bytes
        atr->state = ATR_DONE;
        return 1;
    }
    return 0;
}

int atr_analyze(struct atr *atr, unsigned char data) {
    // Already done?
    if (atr->state == ATR_DONE) return 1;
    atr->bytes_left--;
    // Waiting for more data?
    if (atr->bytes_left > 0) {
        return 0;
    }
    switch (atr->state) {
        case WAIT_T0:
            // Lower nibble of T0 is number of historical bytes
            atr->num_historical_bytes = data & 0xF;
            return handle_t_bits(atr, data);
        case WAIT_TD:
            {
                unsigned char version = data & 0xF;
                if (atr->first_protocol_suggested == 0xFF) {
                    atr->first_protocol_suggested = version;
                }
                // Lower nibble of TD is suggested protocol (if not 15)
                if (version < 15 && version > atr->max_protocol_suggested) {
                    atr->max_protocol_suggested = version;
                }
                return handle_t_bits(atr, data);
            }
        case WAIT_END:
            atr->state = ATR_DONE;
            return 1;
    }
}

void atr_done(struct atr *atr, unsigned *new_proto) {
    if (new_proto && atr->first_protocol_suggested != 0xFF) {
        *new_proto = atr->first_protocol_suggested;
    }
}

void atr_print_state(struct atr *atr) {
    static const char* state_names[] =
        { "WAIT_T0", "WAIT_TD", "WAIT_END", "ATR_DONE" };
    printf("State %s, need %d bytes (%d hist bytes, first proto %d, max proto %d)\n",
        state_names[atr->state], atr->bytes_left, atr->num_historical_bytes,
        atr->first_protocol_suggested, atr->max_protocol_suggested);
}
