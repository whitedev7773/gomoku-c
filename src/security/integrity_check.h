#ifndef INTEGRITY_CHECK_H
#define INTEGRITY_CHECK_H

#include <stdbool.h>
#include <stdint.h>

#define SHA256_HASH_SIZE 32
#define SHA256_HEX_SIZE 65 // 64 chars + null terminator

/**
 * SHA256 해시 구조체
 */
typedef struct
{
    uint8_t hash[SHA256_HASH_SIZE];
} SHA256Hash;

/**
 * 무결성 검증 결과
 */
typedef enum
{
    INTEGRITY_OK = 0,           // 무결성 검증 성공
    INTEGRITY_HASH_MISMATCH,    // 해시 불일치
    INTEGRITY_HASH_FILE_MISSING, // 해시 파일 없음
    INTEGRITY_SELF_READ_ERROR,  // 자가 읽기 오류
    INTEGRITY_HASH_PARSE_ERROR  // 해시 파싱 오류
} IntegrityResult;

/**
 * 실행 파일의 SHA256 해시 계산
 * @param filepath 실행 파일 경로
 * @param out_hash 출력 해시 (32바이트)
 * @return 성공 시 true
 */
bool integrity_calculate_file_hash(const char *filepath, SHA256Hash *out_hash);

/**
 * 저장된 공식 해시 로드
 * @param hash_filepath 해시 파일 경로 (.sha256)
 * @param out_hash 출력 해시
 * @return 성공 시 true
 */
bool integrity_load_official_hash(const char *hash_filepath, SHA256Hash *out_hash);

/**
 * 두 해시 비교
 * @return 동일하면 true
 */
bool integrity_compare_hash(const SHA256Hash *hash1, const SHA256Hash *hash2);

/**
 * 프로그램 시작 시 무결성 검증
 * @param exe_path 실행 파일 경로
 * @return IntegrityResult
 */
IntegrityResult integrity_verify_self(const char *exe_path);

/**
 * 해시를 16진수 문자열로 변환
 * @param hash 해시
 * @param out_hex 출력 문자열 (최소 65바이트)
 */
void integrity_hash_to_hex(const SHA256Hash *hash, char *out_hex);

/**
 * 16진수 문자열을 해시로 변환
 * @param hex 16진수 문자열 (64자)
 * @param out_hash 출력 해시
 * @return 성공 시 true
 */
bool integrity_hex_to_hash(const char *hex, SHA256Hash *out_hash);

/**
 * 무결성 검증 결과를 문자열로 변환
 */
const char *integrity_result_to_string(IntegrityResult result);

#endif // INTEGRITY_CHECK_H
