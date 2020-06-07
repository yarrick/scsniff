#include "session.h"

#include <check.h>

#define TEST_FD (999)
#define TEST_BAUDRATE (9600)

static struct session sess;

static unsigned last_baudrate = 0;
static unsigned baudrate_set = 0;
static void new_baudrate(int fd, unsigned baudrate) {
    ck_assert_uint_eq(fd, TEST_FD);
    baudrate_set++;
    last_baudrate = baudrate;
}

static unsigned char last_data[SESSION_BUFLEN];
static struct packet last_packet;
static void new_packet(struct packet *packet) {
    memcpy(&last_packet, packet, sizeof(last_packet));
    memcpy(last_data, packet->data, packet->data_length);
}

static char last_msg[SESSION_BUFLEN];
static void log_msg(const char *msg) {
    strncpy(last_msg, msg, SESSION_BUFLEN);
}

static void setup() {
    session_init(&sess, new_packet, new_baudrate, log_msg,
                 TEST_FD, TEST_BAUDRATE);
    memset(&last_packet, 0, sizeof(last_packet));
    memset(&last_msg, 0, sizeof(last_msg));
}

static void inject_packet(unsigned char *data, unsigned len) {
    unsigned i;
    for (i = 0; i < len; i++) {
        session_add_byte(&sess, data[i]);
    }
}

// Verify packet was returned, and check result and session state.
// Do as macro to keep line number in test failure messages useful.
#define INJECT_PACKET(arr,res,sess_state) do { \
        inject_packet((arr), sizeof(arr)); \
        ck_assert_uint_eq(last_packet.data_length, sizeof(arr)); \
        ck_assert_mem_eq((arr), last_data, sizeof(arr)); \
        ck_assert_uint_eq(last_packet.result, (res)); \
        ck_assert_uint_eq(sess.curr.state, (sess_state)); \
        memset(&last_packet, 0, sizeof(last_packet)); \
    } while (0)

START_TEST(baudrate_setup)
{
    // Baudrate should be configured as session starts.
    ck_assert_uint_eq(last_baudrate, TEST_BAUDRATE);
}
END_TEST

START_TEST(baudrate_switch_atr)
{
    unsigned char atr[] = { 0x3B, 0xD2, 0x13, 0xFF, 0x10, 0x80, 0x07, 0x14};
    INJECT_PACKET(atr, PACKET_FROM_CARD, IDLE);
    ck_assert_uint_eq(last_baudrate, TEST_BAUDRATE*4);
    ck_assert_str_eq(last_msg,
                     "Switching to 93 ticks per ETU (38400 baud) after ATR");
    ck_assert_uint_eq(sess.curr.protocol_version, 0);
}
END_TEST

START_TEST(baudrate_switch_pps)
{
    unsigned char atr[] = {
        0x3B, 0x7F, 0x18, 0x00, 0x00, 0x43, 0x55, 0x32, 0x69, 0xAA, 0x20,
        0x00, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00 };
    INJECT_PACKET(atr, PACKET_FROM_CARD, IDLE);
    ck_assert_uint_eq(sess.curr.protocol_version, 0);

    unsigned char pps[] = { 0xFF, 0x10, 0x13, 0xFC };
    INJECT_PACKET(pps, PACKET_TO_CARD, PPS);
    INJECT_PACKET(pps, PACKET_FROM_CARD, IDLE);
    ck_assert_uint_eq(last_baudrate, TEST_BAUDRATE*4);
    ck_assert_str_eq(last_msg,
                     "Switching to 93 ticks per ETU (38400 baud) after PPS");
    ck_assert_uint_eq(sess.curr.protocol_version, 0);
}
END_TEST

START_TEST(protocol_switch_atr)
{
    unsigned char atr[] = {
        0x3B, 0xFF, 0x13, 0x00, 0x00, 0x81, 0x31, 0xFE, 0x45, 0x43, 0x44,
        0x32, 0x69, 0xA9, 0x41, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x00, 0x53 };
    INJECT_PACKET(atr, PACKET_FROM_CARD, IDLE);
    ck_assert_str_eq(last_msg, "Switching to protocol T=1 after ATR");
    ck_assert_uint_eq(sess.curr.protocol_version, 1);

    unsigned char t1_data[] = { 0x00, 0xC1, 0x01, 0xFE, 0x3E };
    INJECT_PACKET(t1_data, PACKET_TO_CARD, T1_DATA);
    ck_assert_uint_eq(sess.curr.protocol_version, 1);
}

START_TEST(protocol_switch_pps)
{
    unsigned char atr[] = { 0x3B, 0x80, 0x80, 0x01, 0x01 };
    INJECT_PACKET(atr, PACKET_FROM_CARD, IDLE);
    ck_assert_uint_eq(sess.curr.protocol_version, 0);

    unsigned char pps[] = { 0xFF, 0x11, 0x11, 0xFC };
    INJECT_PACKET(pps, PACKET_TO_CARD, PPS);
    INJECT_PACKET(pps, PACKET_FROM_CARD, IDLE);
    // Baudrate not set again after init.
    ck_assert_uint_eq(baudrate_set, 1);
    ck_assert_str_eq(last_msg, "Switching to protocol T=1 after PPS");
    ck_assert_uint_eq(sess.curr.protocol_version, 1);

    unsigned char t1_data[] = { 0x00, 0xC1, 0x01, 0xFE, 0x3E };
    INJECT_PACKET(t1_data, PACKET_TO_CARD, T1_DATA);
    ck_assert_uint_eq(sess.curr.protocol_version, 1);
}
END_TEST

START_TEST(t0_data_exchange)
{
    unsigned char atr[] = { 0x3B, 0x80, 0x80, 0x01, 0x01 };
    INJECT_PACKET(atr, PACKET_FROM_CARD, IDLE);
    ck_assert_uint_eq(sess.curr.protocol_version, 0);

    unsigned char t0_cmd[] = { 0x00, 0xCA, 0x01, 0x00, 0x02 };
    INJECT_PACKET(t0_cmd, PACKET_TO_CARD, T0_DATA);
    unsigned char t0_ack[] = { 0xCA };
    INJECT_PACKET(t0_ack, PACKET_FROM_CARD, T0_DATA);
    unsigned char t0_data[] = { 0x01, 0x02 };
    INJECT_PACKET(t0_data, PACKET_FROM_CARD, T0_DATA);
    unsigned char t0_status[] = { 0x90, 0x00 };
    INJECT_PACKET(t0_status, PACKET_FROM_CARD, T0_DATA);
}
END_TEST

START_TEST(t1_data_exchange)
{
    unsigned char atr[] = {
        0x3B, 0xFF, 0x13, 0x00, 0x00, 0x81, 0x31, 0xFE, 0x45, 0x43, 0x44,
        0x32, 0x69, 0xA9, 0x41, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x00, 0x53 };
    INJECT_PACKET(atr, PACKET_FROM_CARD, IDLE);
    ck_assert_uint_eq(sess.curr.protocol_version, 1);

    unsigned char t1_cmd[] = { 0x00, 0xC1, 0x01, 0xFE, 0x3E };
    INJECT_PACKET(t1_cmd, PACKET_TO_CARD, T1_DATA);
    unsigned char t1_resp[] = { 0x00, 0xE1, 0x01, 0xFE, 0x1E };
    INJECT_PACKET(t1_resp, PACKET_FROM_CARD, T1_DATA);
}
END_TEST

Suite* session_tests() {
    Suite* suite = suite_create("session");
    TCase* test = tcase_create("session");
    tcase_add_checked_fixture(test, setup, NULL);
    tcase_add_test(test, baudrate_setup);
    tcase_add_test(test, baudrate_switch_atr);
    tcase_add_test(test, baudrate_switch_pps);
    tcase_add_test(test, protocol_switch_atr);
    tcase_add_test(test, protocol_switch_pps);
    tcase_add_test(test, t0_data_exchange);
    tcase_add_test(test, t1_data_exchange);
    suite_add_tcase(suite, test);
    return suite;
}
