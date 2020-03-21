#include "data.h"

#include <string.h>

void data_init(struct data *data) {
    memset(data, 0, sizeof(struct data));
    data->t1_check_len = LRC_XOR;
}

int data_t0_analyze(struct data *data, unsigned char byte) {
    switch (data->t0_state) {
        case COMMAND:
            data->t0_command_bytes_seen++;
            if (data->t0_command_bytes_seen == 2) {
                data->t0_ins = byte;
            } else if (data->t0_command_bytes_seen == 5) {
                // 0x00 means 0x100 for transfers from the card,
                // but 0x00 for transfers to the card.
                // Transfer direction is not known here.
                data->t0_p3_len = byte;
                data->t0_state = PROCEDURE_BYTE;
                return 1;
            }
            break;
        case PROCEDURE_BYTE:
            if (byte == 0x60) return 1;
            if ((byte & 0xF0) == 0x60 || (byte & 0xF0) == 0x90) {
                data->t0_state = SW2;
            }
            if (byte == data->t0_ins) {
                data->t0_state = TRANSFER_ALL;
                return 1;
            }
            if ((byte ^ 0xFF) == data->t0_ins) {
                // Expect a single byte if not all sent already
                if (data->t0_transfer_bytes_seen < data->t0_p3_len) {
                    data->t0_state = TRANSFER_ONE;
                }
                return 1;
            }
            break;
        case SW2:
            // Completion of whole exchange.
            data->t0_state = COMMAND;
            data->t0_command_bytes_seen = 0;
            data->t0_transfer_bytes_seen = 0;
            return 1;
        case TRANSFER_ALL:
            data->t0_transfer_bytes_seen++;
            if (data->t0_p3_len == data->t0_transfer_bytes_seen) {
                data->t0_state = PROCEDURE_BYTE;
                return 1;
            }
            break;
        case TRANSFER_ONE:
            data->t0_transfer_bytes_seen++;
            data->t0_state = PROCEDURE_BYTE;
            return 1;
    }
    return 0;
}

#define T1_PROLOGUE_LEN (3)

int data_t1_analyze(struct data *data, unsigned char byte) {
    data->t1_bytes_seen++;
    if (data->t1_bytes_seen == T1_PROLOGUE_LEN) {
        data->t1_msg_length = T1_PROLOGUE_LEN + byte + data->t1_check_len;
        return 0;
    }
    if (data->t1_msg_length && data->t1_bytes_seen == data->t1_msg_length) {
        // End of block, reset counters for next block
        data->t1_bytes_seen = 0;
        data->t1_msg_length = 0;
        return 1;
    }
    return 0;
}
