#ifndef MEMORY_CHECK_H
#define MEMORY_CHECK_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define MAX_PROTECTED_FUNCTIONS 32

/**
 * 보호할 함수 정보
 */
typedef struct
{
    void *function_ptr;    // 함수 포인터
    uint32_t checksum;     // 함수 체크섬
    size_t size;           // 함수 크기 (바이트)
    const char *name;      // 함수 이름 (디버깅용)
    bool is_protected;     // 보호 활성화 여부
} ProtectedFunction;

/**
 * 메모리 무결성 체크 상태
 */
typedef struct
{
    ProtectedFunction functions[MAX_PROTECTED_FUNCTIONS];
    int function_count;

    uint32_t text_segment_checksum; // .text 세그먼트 체크섬
    void *text_segment_start;       // .text 세그먼트 시작 주소
    size_t text_segment_size;       // .text 세그먼트 크기

    int check_count;      // 총 체크 횟수
    int violation_count;  // 위반 감지 횟수
} MemoryCheckState;

/**
 * 메모리 무결성 체크 초기화
 */
void memory_check_init(MemoryCheckState *state);

/**
 * 함수를 보호 목록에 추가
 * @param state 메모리 체크 상태
 * @param func_ptr 함수 포인터
 * @param func_size 함수 크기 (바이트)
 * @param func_name 함수 이름
 * @return 성공 시 true
 */
bool memory_check_add_protected_function(MemoryCheckState *state, void *func_ptr, size_t func_size, const char *func_name);

/**
 * 특정 함수의 무결성 검증
 * @param state 메모리 체크 상태
 * @param index 함수 인덱스
 * @return 무결성 유지 시 true
 */
bool memory_check_verify_function(MemoryCheckState *state, int index);

/**
 * 모든 보호된 함수의 무결성 검증
 * @param state 메모리 체크 상태
 * @param out_violated_index 위반된 함수 인덱스 (출력, 위반 시)
 * @return 모든 함수 무결성 유지 시 true
 */
bool memory_check_verify_all_functions(MemoryCheckState *state, int *out_violated_index);

/**
 * .text 세그먼트 체크섬 계산 및 저장
 * @param state 메모리 체크 상태
 * @return 성공 시 true
 */
bool memory_check_initialize_text_segment(MemoryCheckState *state);

/**
 * .text 세그먼트 무결성 검증
 * @param state 메모리 체크 상태
 * @return 무결성 유지 시 true
 */
bool memory_check_verify_text_segment(MemoryCheckState *state);

/**
 * 메모리 영역의 CRC32 체크섬 계산
 * @param data 메모리 영역
 * @param size 크기 (바이트)
 * @return CRC32 체크섬
 */
uint32_t memory_check_calculate_crc32(const void *data, size_t size);

/**
 * 함수 크기 추정 (간단한 휴리스틱)
 * @param func_ptr 함수 포인터
 * @return 추정된 함수 크기 (바이트)
 */
size_t memory_check_estimate_function_size(void *func_ptr);

#endif // MEMORY_CHECK_H
