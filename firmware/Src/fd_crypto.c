#include <string.h>
#include "FreeRTOS.h"

#include "mbedtls/pk.h"
#include "mbedtls/platform_util.h"

#include "fd_crypto.h"
#include "macros.h"

typedef
enum storage_state
{
    FD_CRYPT_NO_INIT   = 0x00,
    FD_CRYPT_GENERATED = 0x01,
    FD_CRYPT_IMPORTED  = 0x02,
} storage_state;

#if (MBEDTLS_PK_MAX_PUBKEY_RAW_LEN < 32)
#error "MBEDTLS_PK_MAX_PUBKEY_RAW_LEN is too small"
#endif

struct fd_crypto_storage
{
    storage_state state;
    uint8_t pk_raw_storage[ALIGN(MBEDTLS_PK_MAX_PUBKEY_RAW_LEN, 4)];
};

struct fd_crypto_region
{
    struct fd_crypto_storage storage;

    uint8_t task_stack[FD_CRYPTO_STACK_SIZE];
};

_Static_assert(
    sizeof(struct fd_crypto_region) <= FD_CRYPTO_REGION_SIZE,
    "Crypto region is too small"
);

static
uint8_t fd_crypto_region_memory[FD_CRYPTO_REGION_SIZE]
__attribute__ ((aligned (FD_CRYPTO_REGION_SIZE)));

static inline
struct fd_crypto_region * FDCryptoRegion(void)
{
    return (struct fd_crypto_region *)fd_crypto_region_memory;
}

void FDCryptoClear(void)
{
    mbedtls_platform_zeroize(&fd_crypto_region_memory, 
                       sizeof(fd_crypto_region_memory));
}


void FDCryptoTask(void * arg)
{

}
