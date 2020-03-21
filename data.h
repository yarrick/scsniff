#ifndef SCSNIFF_DATA_H
#define SCSNIFF_DATA_H

// Signaled in lsb of TC1 when using T=1 protocol.
// 0 = LRC (default), 1 = CRC
enum t1_error_checking {
    LRC_XOR = 1,
    CRC_16 = 2,
};

struct data {
    unsigned t1_bytes_seen;
    unsigned t1_msg_length;
    unsigned t1_check_len;
};

void data_init(struct data *data);

int data_t1_analyze(struct data *data, unsigned char byte);

#endif // SCSNIFF_DATA_H
