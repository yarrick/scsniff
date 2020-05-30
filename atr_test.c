#include "atr.h"

#include <check.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define NO_UPDATE (0xFFFF)

static const struct sample {
    unsigned char data[32];
    unsigned len;
    unsigned first_protocol;
    unsigned speed;
} samples[] = {
    { // T0 only and 2 historical bytes.
        .data = {0x3B, 0x02, 0x36, 0x02}, .len = 4
    },
    { // No historical bytes, and no checksum.
        .data = {0x3B, 0x10, 0x16}, .len = 3,
    },
    { // No historical bytes. Checksum included since T=1 signaled.
        .data = {0x3B, 0x90, 0x16, 0x01, 0x87}, .len = 5,
        .first_protocol = 1,
    },
    { // T=0 and T=1 signaled, some historical data and checksum.
        .data = {0x3B, 0x84, 0x80, 0x01, 0x00, 0x81, 0x87, 0x00, 0x03},
        .len = 9,
    },
    { // With TD(2) and TA(3).
        .data = {0x3B, 0xF6, 0x18, 0x00, 0xFF, 0x81, 0x31, 0xFE,
                 0x45, 0x4A, 0x43, 0x4F, 0x50, 0x33, 0x30, 0x0F},
        .len = 16, .first_protocol = 1,
    },
    { // T=0 and T=15 signaled.
        .data = {0x3B, 0x93, 0x95, 0x80, 0x1F, 0xC7, 0x80, 0x31, 0x80, 0x6F},
        .len = 10,
    },
    { // TA(1) and TA(2) supplied, new speed in use.
        .data = {0x3B, 0xD2, 0x13, 0xFF, 0x10, 0x80, 0x07, 0x14},
        .len = 8, .speed = 0x13,
    },
};

START_TEST(parse_sample)
{
    struct atr atr;
    const struct sample *s = &samples[_i];
    int pos;
    unsigned new_proto = 0;
    unsigned new_speed = NO_UPDATE;
    atr_init(&atr);
    for (pos = 0; pos < s->len; pos++) {
        unsigned complete = 0;
        enum result res = atr_analyze(&atr, s->data[pos], &complete);
        if (pos + 1 == s->len) {
            // Last byte, should signal packet from card, and completed.
            ck_assert_uint_eq(res, PACKET_FROM_CARD);
            ck_assert_uint_ne(complete, 0);
        } else {
            // Mid-packet, keep going.
            ck_assert_uint_eq(res, CONTINUE);
            ck_assert_uint_eq(complete, 0);
        }
    }
    atr_result(&atr, &new_proto, &new_speed);
    ck_assert_uint_eq(new_proto, s->first_protocol);
    if (new_speed != NO_UPDATE) {
        ck_assert_uint_eq(new_speed, s->speed);
    }
}
END_TEST

Suite* atr_tests() {
    Suite* suite = suite_create("atr");
    TCase* test = tcase_create("atr");
    tcase_add_loop_test(test, parse_sample, 0, ARRAY_SIZE(samples));
    suite_add_tcase(suite, test);
    return suite;
}
