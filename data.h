#ifndef SCSNIFF_DATA_H
#define SCSNIFF_DATA_H

enum data_t0_state {
    COMMAND = 0,
    PROCEDURE_BYTE,
    SW2,
    TRANSFER_ALL,
    TRANSFER_ONE,
};

// Signaled in lsb of TC1 in ATR when using T=1 protocol.
// 0 = LRC (default), 1 = CRC
enum data_t1_error_checking {
    LRC_XOR = 1,
    CRC_16 = 2,
};

struct data {
    enum data_t0_state t0_state;
    unsigned t0_command_bytes_seen;
    unsigned t0_ins;
    unsigned t0_p3_len;
    unsigned t0_transfer_bytes_seen;

    unsigned t1_bytes_seen;
    unsigned t1_msg_length;
    unsigned t1_check_len;
};

void data_init(struct data *data);

int data_t0_analyze(struct data *data, unsigned char byte);

int data_t1_analyze(struct data *data, unsigned char byte);

#endif // SCSNIFF_DATA_H
