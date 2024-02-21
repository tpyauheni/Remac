#include "testmain.hpp"

static unsigned int TOTAL_TESTS = 0;
static unsigned int PASSED_TESTS = 0;

void test_module(std::string name) {
    std::printf("Testing module '%s'\n", name.c_str());
    std::fflush(stdout);
}

void test_condition(bool condition) {
    ++TOTAL_TESTS;

    if (condition) {
        ++PASSED_TESTS;
        std::printf("Test %u passed\n", TOTAL_TESTS);
    } else {
        std::printf("Test %u failed\n", TOTAL_TESTS);
    }
}

int main() {
    try {
        test_main();
    } catch (...) {
        std::printf("*Interrupted with exception*\n");
    }

    if (TOTAL_TESTS == 0) {
        std::printf("No tests was successfully finished\n");
    } else {
        std::printf("Test results: %u / %u (%.2f%%)\n", PASSED_TESTS, TOTAL_TESTS, (float)PASSED_TESTS / (float)TOTAL_TESTS * 100.0F);
    }
}
