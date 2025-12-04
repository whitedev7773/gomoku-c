#include "test_framework.h"

// 전역 테스트 통계
TestStats g_test_stats = {0, 0, 0};

void test_init(void) {
    g_test_stats.total = 0;
    g_test_stats.passed = 0;
    g_test_stats.failed = 0;
}

void test_summary(void) {
    printf("\n");
    printf("===========================================\n");
    printf("Test Summary\n");
    printf("===========================================\n");
    printf("Total:  %d\n", g_test_stats.total);
    printf("Passed: %d\n", g_test_stats.passed);
    printf("Failed: %d\n", g_test_stats.failed);
    printf("===========================================\n");

    if (g_test_stats.failed == 0) {
        printf("ALL TESTS PASSED! ✓\n");
    } else {
        printf("SOME TESTS FAILED! ✗\n");
    }
    printf("===========================================\n\n");
}
