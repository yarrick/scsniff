#include "data.h"

#include <string.h>

void data_init(struct data *data) {
    memset(data, 0, sizeof(struct data));
    data->t1_check_len = LRC_XOR;
}

static enum result data_transfer_direction(struct data *data) {
    switch (data->t0_ins) {
        // First data packet (if any) will be to the card.
        // Any return data will be read with GET RESPONSE.
        case 0x0E: case 0x0F: // ERASE BINARY
        case 0x20: case 0x21: // VERIFY
        case 0x22: // MANAGE SECURITY ENVIRONMENT/KEY DERIVATION
        case 0x24: // CHANGE REFERENCE DATA
        case 0x26: // DISABLE VERIFICATION REQUIREMENT
        case 0x28: // ENABLE VERIFICATION REQUIREMENT
        case 0x2C: // RESET RETRY COUNTER
        case 0x82: // EXTERNAL or MUTUAL AUTHENTICATE
        case 0x86: case 0x87: // GENERAL AUTHENTICATE
        case 0x88: // INTERNAL AUTHENTICATE
        case 0xA1: // SEARCH BINARY
        case 0xA2: // SEARCH RECORD
        case 0xB1: // READ BINARY
        case 0xB3: // READ RECORD
        case 0xC2: case 0xC3: // ENVELOPE
        case 0xCB: // GET DATA
        case 0xD0: case 0xD1: // WRITE BINARY
        case 0xD2: // WRITE RECORD
        case 0xD6: case 0xD7: // UPDATE BINARY
        case 0xDA: case 0xDB: // PUT DATA
        case 0xDC: case 0xDD: // UPDATE RECORD
        case 0xE2: // APPEND RECORD
            return PACKET_TO_CARD;

        // First data packet (if any) will be from the card.
        case 0x70: // MANAGE CHANNEL
        case 0x84: // GET CHALLENGE
        case 0xB0: // READ BINARY
        case 0xB2: // READ RECORD
        case 0xC0: // GET RESPONSE
        case 0xCA: // GET DATA
            return PACKET_FROM_CARD;

        case 0xA4: // SELECT
            // ISO 7816-4 Table 39: P1 meaning for SELECT command.
            switch (data->t0_p1) {
                case 0x01: // Select child DF (data = DF identifier)
                case 0x02: // Select EF under current DF (data = EF identifier)
                case 0x04: // Select by DF name (data = app identifier)
                case 0x08: // Select from the MF (data = Path without MF id)
                case 0x09: // Select from the current DF
                           // (data = path without the current DF identifier)
                    return PACKET_TO_CARD;

                case 0x03: // Select parent DF of the current DF (data absent)
                    return PACKET_FROM_CARD;
            }
            break;
    }
    return PACKET_UNKNOWN;
}

enum result data_t0_analyze(struct data *data, unsigned char byte) {
    switch (data->t0_state) {
        case COMMAND:
            data->t0_command_bytes_seen++;
            if (data->t0_command_bytes_seen == 2) {
                data->t0_ins = byte;
            } else if (data->t0_command_bytes_seen == 3) {
                data->t0_p1 = byte;
            } else if (data->t0_command_bytes_seen == 4) {
                data->t0_p2 = byte;
            } else if (data->t0_command_bytes_seen == 5) {
                // 0x00 means 0x100 for transfers from the card,
                // but 0x00 for transfers to the card.
                // Transfer direction is not known here.
                data->t0_p3_len = byte;
                data->t0_state = PROCEDURE_BYTE;
                return PACKET_TO_CARD;
            }
            break;
        case PROCEDURE_BYTE:
            if (byte == 0x60) return PACKET_FROM_CARD;
            if ((byte & 0xF0) == 0x60 || (byte & 0xF0) == 0x90) {
                data->t0_state = SW2;
            }
            if (byte == data->t0_ins) {
                data->t0_state = TRANSFER_ALL;
                return PACKET_FROM_CARD;
            }
            if ((byte ^ 0xFF) == data->t0_ins) {
                // Expect a single byte if not all sent already
                if (data->t0_transfer_bytes_seen < data->t0_p3_len) {
                    data->t0_state = TRANSFER_ONE;
                }
                return PACKET_FROM_CARD;
            }
            break;
        case SW2:
            // Completion of whole exchange.
            data->t0_state = COMMAND;
            data->t0_command_bytes_seen = 0;
            data->t0_transfer_bytes_seen = 0;
            return PACKET_FROM_CARD;
        case TRANSFER_ALL:
            data->t0_transfer_bytes_seen++;
            if (data->t0_p3_len == data->t0_transfer_bytes_seen) {
                data->t0_state = PROCEDURE_BYTE;
                return data_transfer_direction(data);
            }
            break;
        case TRANSFER_ONE:
            data->t0_transfer_bytes_seen++;
            data->t0_state = PROCEDURE_BYTE;
            return data_transfer_direction(data);
    }
    return CONTINUE;
}

#define T1_PROLOGUE_LEN (3)

enum result data_t1_analyze(struct data *data, unsigned char byte) {
    data->t1_bytes_seen++;
    if (data->t1_bytes_seen == T1_PROLOGUE_LEN) {
        data->t1_msg_length = T1_PROLOGUE_LEN + byte + data->t1_check_len;
        return CONTINUE;
    }
    if (data->t1_msg_length && data->t1_bytes_seen == data->t1_msg_length) {
        // End of block, reset counters for next block
        enum result res = PACKET_TO_CARD;
        if (data->t1_direction_from_card) res = PACKET_FROM_CARD;
        data->t1_bytes_seen = 0;
        data->t1_msg_length = 0;
        // Flip direction for next packet
        data->t1_direction_from_card ^= 1;
        return res;
    }
    return CONTINUE;
}
