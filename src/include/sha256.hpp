
#ifndef SHA256_HPP
#define SHA256_HPP

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    unsigned long long bitlen;
    uint32_t state[8];
} sha256_ctx_t;

void sha256_init(sha256_ctx_t *ctx);
void sha256_update(sha256_ctx_t *ctx, const uint8_t data[], size_t len);
void sha256_final(sha256_ctx_t *ctx, uint8_t hash[]);

int sha256_file(const char *filepath, char out_hex[65]);

#ifdef __cplusplus
}
#endif

#endif 
