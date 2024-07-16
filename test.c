#include <check.h>

extern Suite *atr_tests(void);
extern Suite *data_tests(void);
extern Suite *session_tests(void);

int main(void) {
    SRunner *runner;
    int failed;

    runner = srunner_create(atr_tests());
    srunner_add_suite(runner, data_tests());
    srunner_add_suite(runner, session_tests());
    srunner_run_all(runner, CK_NORMAL);
    failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return failed;
}
