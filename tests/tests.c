#include "../ext/tinytest.h"
#include "../ext/tinytest_macros.h"

#include <stdio.h>
#include <stdlib.h>

#include "../src/util.h"

void test_is_valid_module_name(void *ptr) {
    tt_int_op(is_valid_module_name("validmodule"), ==, 1);

    tt_int_op(is_valid_module_name("/usr/lib/lua/invalid.lua"), ==, 0);
    tt_int_op(is_valid_module_name("../luasrc/api.lua"), ==, 0);
    tt_int_op(is_valid_module_name("subdir/file.lua"), ==, 0);

end:
    ;
}

struct testcase_t censorscope_tests[] = {
    { "is_valid_module_name", test_is_valid_module_name },

    END_OF_TESTCASES
};

struct testgroup_t groups[] = {
    { "censorscope/", censorscope_tests },

    END_OF_GROUPS
};

int main(int argc, const char **argv) {
    return tinytest_main(argc, argv, groups);
}
