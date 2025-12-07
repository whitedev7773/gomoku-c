#define OPENSSL_API_COMPAT 0x10100000L
#define OPENSSL_NO_DEPRECATED

#include "integrity_check.h"
#include "security_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

bool integrity_calculate_file_hash(const char *filepath, SHA256Hash *out_hash)
{
    if (!filepath || !out_hash)
    {
        return false;
    }

    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        return false;
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
    {
        fclose(file);
        return false;
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1)
    {
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return false;
    }

    unsigned char buffer[8192];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        if (EVP_DigestUpdate(mdctx, buffer, bytes_read) != 1)
        {
            EVP_MD_CTX_free(mdctx);
            fclose(file);
            return false;
        }
    }

    unsigned int hash_len;
    if (EVP_DigestFinal_ex(mdctx, out_hash->hash, &hash_len) != 1 || hash_len != SHA256_HASH_SIZE)
    {
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return false;
    }

    EVP_MD_CTX_free(mdctx);
    fclose(file);

    return true;
}

bool integrity_load_official_hash(const char *hash_filepath, SHA256Hash *out_hash)
{
    if (!hash_filepath || !out_hash)
    {
        return false;
    }

    FILE *file = fopen(hash_filepath, "r");
    if (!file)
    {
        return false;
    }

    char hex_string[SHA256_HEX_SIZE];
    if (fgets(hex_string, sizeof(hex_string), file) == NULL)
    {
        fclose(file);
        return false;
    }

    fclose(file);

    // 개행 문자 제거
    hex_string[strcspn(hex_string, "\r\n")] = '\0';

    return integrity_hex_to_hash(hex_string, out_hash);
}

bool integrity_compare_hash(const SHA256Hash *hash1, const SHA256Hash *hash2)
{
    if (!hash1 || !hash2)
    {
        return false;
    }

    return memcmp(hash1->hash, hash2->hash, SHA256_HASH_SIZE) == 0;
}

IntegrityResult integrity_verify_self(const char *exe_path)
{
    if (!exe_path)
    {
        return INTEGRITY_SELF_READ_ERROR;
    }

    // 실행 파일의 해시 계산
    SHA256Hash current_hash;
    if (!integrity_calculate_file_hash(exe_path, &current_hash))
    {
        security_log_critical("Failed to calculate self hash for: %s", exe_path);
        return INTEGRITY_SELF_READ_ERROR;
    }

    // 공식 해시 로드 (.sha256 파일)
    char hash_filepath[512];
    snprintf(hash_filepath, sizeof(hash_filepath), "%s.sha256", exe_path);

    SHA256Hash official_hash;
    if (!integrity_load_official_hash(hash_filepath, &official_hash))
    {
        security_log_warning("Official hash file not found: %s", hash_filepath);
        return INTEGRITY_HASH_FILE_MISSING;
    }

    // 해시 비교
    if (!integrity_compare_hash(&current_hash, &official_hash))
    {
        char current_hex[SHA256_HEX_SIZE];
        char official_hex[SHA256_HEX_SIZE];
        integrity_hash_to_hex(&current_hash, current_hex);
        integrity_hash_to_hex(&official_hash, official_hex);

        security_log_integrity_violation("Hash mismatch!");
        security_log_critical("Current:  %s", current_hex);
        security_log_critical("Official: %s", official_hex);

        return INTEGRITY_HASH_MISMATCH;
    }

    security_log_write(SEC_LOG_INFO, SEC_EVENT_INTEGRITY_CHECK,
                       "Integrity verification passed for: %s", exe_path);

    return INTEGRITY_OK;
}

void integrity_hash_to_hex(const SHA256Hash *hash, char *out_hex)
{
    if (!hash || !out_hex)
    {
        return;
    }

    for (int i = 0; i < SHA256_HASH_SIZE; i++)
    {
        snprintf(out_hex + (i * 2), 3, "%02x", hash->hash[i]);
    }
    out_hex[SHA256_HEX_SIZE - 1] = '\0';
}

bool integrity_hex_to_hash(const char *hex, SHA256Hash *out_hash)
{
    if (!hex || !out_hash)
    {
        return false;
    }

    size_t hex_len = strlen(hex);
    if (hex_len != SHA256_HASH_SIZE * 2)
    {
        return false;
    }

    for (int i = 0; i < SHA256_HASH_SIZE; i++)
    {
        unsigned int byte_val;
        if (sscanf(hex + (i * 2), "%2x", &byte_val) != 1)
        {
            return false;
        }
        out_hash->hash[i] = (uint8_t)byte_val;
    }

    return true;
}

const char *integrity_result_to_string(IntegrityResult result)
{
    switch (result)
    {
    case INTEGRITY_OK:
        return "OK";
    case INTEGRITY_HASH_MISMATCH:
        return "Hash Mismatch";
    case INTEGRITY_HASH_FILE_MISSING:
        return "Hash File Missing";
    case INTEGRITY_SELF_READ_ERROR:
        return "Self Read Error";
    case INTEGRITY_HASH_PARSE_ERROR:
        return "Hash Parse Error";
    default:
        return "Unknown";
    }
}
