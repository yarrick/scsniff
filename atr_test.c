#include "atr.h"

#include <check.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

static const struct sample {
    unsigned char data[32];
    unsigned len;
    unsigned max_protocol;
} samples[] = {
    { // T0 only and 2 historical bytes.
        .data = {0x3B, 0x02, 0x36, 0x02}, .len = 4
    },
    { // No historical bytes, and no checksum.
        .data = {0x3B, 0x10, 0x16 }, .len = 3,
    },
    { // No historical bytes. Checksum included since T=1 signaled.
        .data = {0x3B, 0x90, 0x16, 0x01, 0x87}, .len = 5,
        .max_protocol = 1,
    },
};

START_TEST(parse_sample)
{
    struct atr atr;
    const struct sample *s = &samples[_i];
    int pos;
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
    ck_assert_uint_eq(atr.max_protocol_suggested, s->max_protocol);
}
END_TEST

int main() {
    SRunner *runner;
    int failed;
    Suite* suite = suite_create("atr");
    TCase* test = tcase_create("atr");
    tcase_add_loop_test(test, parse_sample, 0, ARRAY_SIZE(samples));
    suite_add_tcase(suite, test);

    runner = srunner_create(suite);
    srunner_run_all(runner, CK_NORMAL);
    failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return failed;
}
