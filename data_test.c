#include "data.h"

#include <check.h>

#define T0_BYTE(data,byte,exp) ck_assert_uint_eq( \
    data_t0_analyze((data),(byte)), (exp))

START_TEST(t0_empty_ack)
{
    struct data data;
    data_init(&data);
    // Get data, 4 bytes
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0xCA, CONTINUE);
    T0_BYTE(&data, 0x01, CONTINUE);
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0x04, T0_DATA_CMD_HEADER);
    // ACK
    T0_BYTE(&data, 0xCA, T0_DATA_ACK);
    // Remaining bytes
    T0_BYTE(&data, 0x01, CONTINUE);
    T0_BYTE(&data, 0x02, CONTINUE);
    T0_BYTE(&data, 0x03, CONTINUE);
    T0_BYTE(&data, 0x04, T0_DATA_RESP_BODY);
    // Extra ACKs, no data remaining
    T0_BYTE(&data, 0xCA, T0_DATA_ACK);
    T0_BYTE(&data, 0xCA, T0_DATA_ACK);
    T0_BYTE(&data, 0xCA, T0_DATA_ACK);
    // Status bytes
    T0_BYTE(&data, 0x90, CONTINUE);
    T0_BYTE(&data, 0x00, T0_DATA_RESP_SW);
}
END_TEST

START_TEST(t0_null_procedure_byte)
{
    struct data data;
    data_init(&data);
    // Get data, 4 bytes
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0xCA, CONTINUE);
    T0_BYTE(&data, 0x01, CONTINUE);
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0x04, T0_DATA_CMD_HEADER);
    // NULL procedure byte
    T0_BYTE(&data, 0x60, T0_DATA_NULL);
    T0_BYTE(&data, 0x60, T0_DATA_NULL);
    T0_BYTE(&data, 0x60, T0_DATA_NULL);
    T0_BYTE(&data, 0x60, T0_DATA_NULL);
    // ACK
    T0_BYTE(&data, 0xCA, T0_DATA_ACK);
    // Remaining bytes
    T0_BYTE(&data, 0x01, CONTINUE);
    T0_BYTE(&data, 0x02, CONTINUE);
    T0_BYTE(&data, 0x03, CONTINUE);
    T0_BYTE(&data, 0x04, T0_DATA_RESP_BODY);
    // NULL procedure byte
    T0_BYTE(&data, 0x60, T0_DATA_NULL);
    T0_BYTE(&data, 0x60, T0_DATA_NULL);
    // Status bytes
    T0_BYTE(&data, 0x90, CONTINUE);
    T0_BYTE(&data, 0x00, T0_DATA_RESP_SW);
}
END_TEST

START_TEST(t0_single_byte_transfer)
{
    struct data data;
    data_init(&data);
    // Get data, 4 bytes
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0xCA, CONTINUE);
    T0_BYTE(&data, 0x01, CONTINUE);
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0x04, T0_DATA_CMD_HEADER);

    // Single-byte ACK
    T0_BYTE(&data, 0xFF ^ 0xCA, T0_DATA_ACK);
    // One byte
    T0_BYTE(&data, 0x01, T0_DATA_RESP_BODY);
    // Repeat
    T0_BYTE(&data, 0xFF ^ 0xCA, T0_DATA_ACK);
    T0_BYTE(&data, 0x02, T0_DATA_RESP_BODY);
    T0_BYTE(&data, 0xFF ^ 0xCA, T0_DATA_ACK);
    T0_BYTE(&data, 0x03, T0_DATA_RESP_BODY);
    T0_BYTE(&data, 0xFF ^ 0xCA, T0_DATA_ACK);
    T0_BYTE(&data, 0x04, T0_DATA_RESP_BODY);
    // Extra single-byte acks with no data
    T0_BYTE(&data, 0xFF ^ 0xCA, T0_DATA_ACK);
    T0_BYTE(&data, 0xFF ^ 0xCA, T0_DATA_ACK);
    // Status bytes
    T0_BYTE(&data, 0x90, CONTINUE);
    T0_BYTE(&data, 0x00, T0_DATA_RESP_SW);
}
END_TEST

START_TEST(t0_get_response)
{
    struct data data;
    data_init(&data);
    // Select DF, 4 bytes
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0xA4, CONTINUE);
    T0_BYTE(&data, 0x04, CONTINUE);
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0x04, T0_DATA_CMD_HEADER);
    // ACK
    T0_BYTE(&data, 0xA4, T0_DATA_ACK);
    // DF name
    T0_BYTE(&data, 'T', CONTINUE);
    T0_BYTE(&data, 'e', CONTINUE);
    T0_BYTE(&data, 's', CONTINUE);
    T0_BYTE(&data, 't', T0_DATA_CMD_BODY);
    // Status bytes
    T0_BYTE(&data, 0x61, CONTINUE);
    T0_BYTE(&data, 0x03, T0_DATA_RESP_SW);

    // Get response
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0xC0, CONTINUE);
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0x03, T0_DATA_CMD_HEADER);
    // ACK
    T0_BYTE(&data, 0xC0, T0_DATA_ACK);
    // Response bytes
    T0_BYTE(&data, 'Y', CONTINUE);
    T0_BYTE(&data, 'a', CONTINUE);
    T0_BYTE(&data, 'y', T0_DATA_RESP_BODY);
    // Status bytes
    T0_BYTE(&data, 0x90, CONTINUE);
    T0_BYTE(&data, 0x00, T0_DATA_RESP_SW);
}
END_TEST

START_TEST(t0_unknown_direction)
{
    struct data data;
    data_init(&data);
    // Unknown command with 4 bytes payload
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0x23, CONTINUE);
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0x04, T0_DATA_CMD_HEADER);
    // ACK
    T0_BYTE(&data, 0x23, T0_DATA_ACK);
    // Payload bytes
    T0_BYTE(&data, 0x01, CONTINUE);
    T0_BYTE(&data, 0x02, CONTINUE);
    T0_BYTE(&data, 0x03, CONTINUE);
    T0_BYTE(&data, 0x04, T0_DATA_UNK_BODY);
    // Status bytes
    T0_BYTE(&data, 0x90, CONTINUE);
    T0_BYTE(&data, 0x00, T0_DATA_RESP_SW);
}
END_TEST

Suite* data_tests(void) {
    Suite* suite = suite_create("data");
    TCase* test = tcase_create("data");
    tcase_add_test(test, t0_empty_ack);
    tcase_add_test(test, t0_null_procedure_byte);
    tcase_add_test(test, t0_single_byte_transfer);
    tcase_add_test(test, t0_get_response);
    tcase_add_test(test, t0_unknown_direction);
    suite_add_tcase(suite, test);
    return suite;
}
