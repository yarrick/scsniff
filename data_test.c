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
    T0_BYTE(&data, 0x04, PACKET_TO_CARD);
    // ACK
    T0_BYTE(&data, 0xCA, PACKET_FROM_CARD);
    // Remaining bytes
    T0_BYTE(&data, 0x01, CONTINUE);
    T0_BYTE(&data, 0x02, CONTINUE);
    T0_BYTE(&data, 0x03, CONTINUE);
    T0_BYTE(&data, 0x04, PACKET_FROM_CARD);
    // Extra ACKs, no data remaining
    T0_BYTE(&data, 0xCA, PACKET_FROM_CARD);
    T0_BYTE(&data, 0xCA, PACKET_FROM_CARD);
    T0_BYTE(&data, 0xCA, PACKET_FROM_CARD);
    // Status bytes
    T0_BYTE(&data, 0x90, CONTINUE);
    T0_BYTE(&data, 0x00, PACKET_FROM_CARD);
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
    T0_BYTE(&data, 0x04, PACKET_TO_CARD);
    // NULL procedure byte
    T0_BYTE(&data, 0x60, PACKET_FROM_CARD);
    T0_BYTE(&data, 0x60, PACKET_FROM_CARD);
    T0_BYTE(&data, 0x60, PACKET_FROM_CARD);
    T0_BYTE(&data, 0x60, PACKET_FROM_CARD);
    // ACK
    T0_BYTE(&data, 0xCA, PACKET_FROM_CARD);
    // Remaining bytes
    T0_BYTE(&data, 0x01, CONTINUE);
    T0_BYTE(&data, 0x02, CONTINUE);
    T0_BYTE(&data, 0x03, CONTINUE);
    T0_BYTE(&data, 0x04, PACKET_FROM_CARD);
    // NULL procedure byte
    T0_BYTE(&data, 0x60, PACKET_FROM_CARD);
    T0_BYTE(&data, 0x60, PACKET_FROM_CARD);
    // Status bytes
    T0_BYTE(&data, 0x90, CONTINUE);
    T0_BYTE(&data, 0x00, PACKET_FROM_CARD);
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
    T0_BYTE(&data, 0x04, PACKET_TO_CARD);

    // Single-byte ACK
    T0_BYTE(&data, 0xFF ^ 0xCA, PACKET_FROM_CARD);
    // One byte
    T0_BYTE(&data, 0x01, PACKET_FROM_CARD);
    // Repeat
    T0_BYTE(&data, 0xFF ^ 0xCA, PACKET_FROM_CARD);
    T0_BYTE(&data, 0x02, PACKET_FROM_CARD);
    T0_BYTE(&data, 0xFF ^ 0xCA, PACKET_FROM_CARD);
    T0_BYTE(&data, 0x03, PACKET_FROM_CARD);
    T0_BYTE(&data, 0xFF ^ 0xCA, PACKET_FROM_CARD);
    T0_BYTE(&data, 0x04, PACKET_FROM_CARD);
    // Extra single-byte acks with no data
    T0_BYTE(&data, 0xFF ^ 0xCA, PACKET_FROM_CARD);
    T0_BYTE(&data, 0xFF ^ 0xCA, PACKET_FROM_CARD);
    // Status bytes
    T0_BYTE(&data, 0x90, CONTINUE);
    T0_BYTE(&data, 0x00, PACKET_FROM_CARD);
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
    T0_BYTE(&data, 0x04, PACKET_TO_CARD);
    // ACK
    T0_BYTE(&data, 0xA4, PACKET_FROM_CARD);
    // DF name
    T0_BYTE(&data, 'T', CONTINUE);
    T0_BYTE(&data, 'e', CONTINUE);
    T0_BYTE(&data, 's', CONTINUE);
    T0_BYTE(&data, 't', PACKET_TO_CARD);
    // Status bytes
    T0_BYTE(&data, 0x61, CONTINUE);
    T0_BYTE(&data, 0x03, PACKET_FROM_CARD);

    // Get response
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0xC0, CONTINUE);
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0x00, CONTINUE);
    T0_BYTE(&data, 0x03, PACKET_TO_CARD);
    // ACK
    T0_BYTE(&data, 0xC0, PACKET_FROM_CARD);
    // Response bytes
    T0_BYTE(&data, 'Y', CONTINUE);
    T0_BYTE(&data, 'a', CONTINUE);
    T0_BYTE(&data, 'y', PACKET_FROM_CARD);
    // Status bytes
    T0_BYTE(&data, 0x90, CONTINUE);
    T0_BYTE(&data, 0x00, PACKET_FROM_CARD);
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
    T0_BYTE(&data, 0x04, PACKET_TO_CARD);
    // ACK
    T0_BYTE(&data, 0x23, PACKET_FROM_CARD);
    // Payload bytes
    T0_BYTE(&data, 0x01, CONTINUE);
    T0_BYTE(&data, 0x02, CONTINUE);
    T0_BYTE(&data, 0x03, CONTINUE);
    T0_BYTE(&data, 0x04, PACKET_UNKNOWN);
    // Status bytes
    T0_BYTE(&data, 0x90, CONTINUE);
    T0_BYTE(&data, 0x00, PACKET_FROM_CARD);
}
END_TEST

Suite* data_tests() {
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
