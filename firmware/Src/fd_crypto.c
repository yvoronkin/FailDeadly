#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

#include "mbedtls/pk.h"
#include "mbedtls/platform_util.h"

#include "fd_crypto.h"
#include "macros.h"


#if (MBEDTLS_PK_MAX_PUBKEY_RAW_LEN < 32)
#error "MBEDTLS_PK_MAX_PUBKEY_RAW_LEN is too small"
#endif


typedef
enum storage_state
{
    FD_CRYPT_NO_INIT   = 0x00,
    FD_CRYPT_GENERATED = 0x01,
    FD_CRYPT_IMPORTED  = 0x02,
} storage_state;


struct fd_crypto_storage
{
    storage_state state;
    uint8_t pk_raw_storage[
        ALIGN(MBEDTLS_PK_MAX_PUBKEY_RAW_LEN, 4)
    ];
};

struct fd_crypto_region
{
    struct fd_crypto_storage storage;
    uint8_t noUse_mem_gap[FD_CRYPTO_SUBREGION_SIZE];
};

static
struct fd_crypto_region fd_crypto_region_memory
__attribute__ ((aligned (512)));

static
portSTACK_TYPE crypto_task_stack[FD_CRYPTO_STACK_WORDS]
__attribute__((aligned(FD_CRYPTO_STACK_WORDS * 4)));

static const 
MemoryRegion_t keys = {
    &fd_crypto_region_memory.storage,
    sizeof(fd_crypto_region_memory.storage),
    portMPU_REGION_READ_WRITE | portMPU_REGION_EXECUTE_NEVER
};

void FDCryptoClear(void)
{
    mbedtls_platform_zeroize(&fd_crypto_region_memory, 
                       sizeof(fd_crypto_region_memory));
}

static
void FDCryptoTask(void * arg)
{
    (void)arg;
    while (1)
    {
        vTaskDelay(10);
    }
}

void FDCryptoTaskInit(void)
{
    BaseType_t res;

    const
    TaskParameters_t param = {
        FDCryptoTask,
        "Crypto",
        FD_CRYPTO_STACK_WORDS,
        NULL,
        6,
        crypto_task_stack,
        {
            keys,
        },
    };

    res = xTaskCreateRestricted(&param, NULL);
    if (res != pdPASS)
    {
        Error_Handler();
    }
}
