#include "data.h"

#include <string.h>

void data_init(struct data *data) {
    memset(data, 0, sizeof(struct data));
    data->t1_check_len = LRC_XOR;
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
