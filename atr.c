#include "atr.h"

#include <stdio.h>
#include <string.h>

#define NO_VALUE (0xFFFF)

void atr_init(struct atr *atr) {
    memset(atr, 0, sizeof(struct atr));
    // State WAIT_T0, need two bytes
    atr->bytes_left = 2;
    atr->first_protocol_suggested = NO_VALUE;
    atr->ta1_value = NO_VALUE;
    atr->ta2_value = NO_VALUE;
}

static int handle_t_bits(struct atr *atr, unsigned char data, unsigned *complete) {
    unsigned t_bytes = __builtin_popcount(data & 0xF0);
    if (data & 0x80) {
        // Another TD byte coming
        atr->bytes_left = t_bytes;
        atr->state = WAIT_TD;
        if (data & 0x10) atr->state = WAIT_TA_TD;
    } else {
        // T bytes and historical data left
        atr->bytes_left = t_bytes + atr->num_historical_bytes;
        // Expect TCK byte if higher protocol mentioned
        if (atr->latest_protocol > 0) atr->bytes_left++;
        atr->state = WAIT_END;
        if (data & 0x10) atr->state = WAIT_TA_END;
    }
    if (atr->bytes_left == 0) {
        // No more T bytes or historical bytes
        atr->state = ATR_DONE;
        if (complete) *complete = 1;
        return ANSWER_TO_RESET;
    }
    return CONTINUE;
}

static void handle_ta_byte(struct atr *atr, unsigned char data) {
    switch (atr->t_cycle) {
    case 1:
        // Store TA1 in case TA2 signals it should be used directly.
        atr->ta1_value = data;
        break;
    case 2:
        atr->ta2_value = data;
        break;
    }
}

enum result atr_analyze(struct atr *atr, unsigned char data, unsigned *complete) {
    // Already done?
    if (atr->state == ATR_DONE) return STATE_ERROR;
    atr->bytes_left--;
    // Capture TA byte
    if (atr->state == WAIT_TA_TD) {
        handle_ta_byte(atr, data);
        atr->state = WAIT_TD;
    }
    if (atr->state == WAIT_TA_END) {
        handle_ta_byte(atr, data);
        atr->state = WAIT_END;
    }
    // Waiting for more data?
    if (atr->bytes_left > 0) {
        return CONTINUE;
    }
    switch (atr->state) {
        case WAIT_T0:
            // Lower nibble of T0 is number of historical bytes
            atr->num_historical_bytes = data & 0xF;
            // Got bits for TA1, TB1 etc
            atr->t_cycle = 1;
            return handle_t_bits(atr, data, complete);
        case WAIT_TD:
            {
                unsigned char version = data & 0xF;
                // Got bits for next set of T bytes
                atr->t_cycle++;
                if (atr->first_protocol_suggested == NO_VALUE) {
                    atr->first_protocol_suggested = version;
                }
                // Lower nibble of TD is protocol (15=global settings)
                if (version > atr->latest_protocol) {
                    atr->latest_protocol = version;
                }
                return handle_t_bits(atr, data, complete);
            }
        case WAIT_END:
            atr->state = ATR_DONE;
            if (complete) *complete = 1;
            return ANSWER_TO_RESET;
        default:
            return STATE_ERROR;
    }
}

void atr_result(struct atr *atr, unsigned *new_proto, unsigned *new_speed) {
    if (new_proto && atr->first_protocol_suggested != NO_VALUE) {
        *new_proto = atr->first_protocol_suggested;
    }
    if (new_speed && atr->ta1_value != NO_VALUE) {
        if (atr->ta2_value != NO_VALUE && (atr->ta2_value & 0x10) == 0) {
            // Switch if TA2 bit 5 is cleared.
            *new_speed = atr->ta1_value;
        }
    }
}
