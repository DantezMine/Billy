#define VANGO_TEST_ROOT
#include <vangotest/casserts.h>

vango_test(basic_math) {
    int a = 10;
    vg_assert_eq(a, 10);
}

vango_test_main(
        vango_test_reg(basic_math);
)
