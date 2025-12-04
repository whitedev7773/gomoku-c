#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// 테스트 통계
typedef struct {
    int total;
    int passed;
    int failed;
} TestStats;

// 전역 테스트 통계
extern TestStats g_test_stats;

// 테스트 초기화
void test_init(void);

// 테스트 결과 출력
void test_summary(void);

// Assert 매크로
#define TEST_ASSERT(condition, message) \
    do { \
        g_test_stats.total++; \
        if (!(condition)) { \
            g_test_stats.failed++; \
            printf("  [FAIL] %s:%d - %s\n", __FILE__, __LINE__, message); \
        } else { \
            g_test_stats.passed++; \
            printf("  [PASS] %s\n", message); \
        } \
    } while(0)

#define TEST_ASSERT_EQ(actual, expected, message) \
    do { \
        g_test_stats.total++; \
        if ((actual) != (expected)) { \
            g_test_stats.failed++; \
            printf("  [FAIL] %s:%d - %s (expected: %d, got: %d)\n", \
                   __FILE__, __LINE__, message, (int)(expected), (int)(actual)); \
        } else { \
            g_test_stats.passed++; \
            printf("  [PASS] %s\n", message); \
        } \
    } while(0)

#define TEST_ASSERT_TRUE(condition, message) \
    TEST_ASSERT((condition), message)

#define TEST_ASSERT_FALSE(condition, message) \
    TEST_ASSERT(!(condition), message)

#define TEST_SUITE(name) \
    printf("\n=== Test Suite: %s ===\n", name)

#define TEST_CASE(name) \
    printf("\n--- Test Case: %s ---\n", name)

#endif // TEST_FRAMEWORK_H
