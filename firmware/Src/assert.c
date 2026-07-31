#include <stdint.h>
#include <string.h>

#include "stm32_assert.h"
#include "main.h"

#include "macros.h"
#include "fd_log.h"

#ifdef USE_FULL_ASSERT

static const
uint32_t magic = 0xDEADBEEF;

typedef
enum {
    UART_FAIL = 0x01,
    FLUSHED   = 0x02,
} ram_assert_state;

typedef
struct {
    const char * file;
    uint32_t line;
    uint32_t magic;
    ram_assert_state state : 8;
} assert_info_t;

volatile 
assert_info_t g_assert_info;

int u32_to_ascii(char *out, size_t out_size, uint32_t x) {
    // max uint32_t is 4294967295 => 10 digits + '\0'
    if (!out || out_size < 2) return -1;

    char buf[10];
    size_t i = 0;

    if (x == 0) {
        if (out_size < 2) return -1;
        out[0] = '0';
        out[1] = '\0';
        return 0;
    }

    while (x > 0 && i < sizeof(buf)) {
        buf[i++] = (char)('0' + (x % 10u));
        x /= 10u;
    }

    if (i + 1 > out_size) return -1;

    // reverse into out
    for (size_t j = 0; j < i; j++) {
        out[j] = buf[i - 1 - j];
    }
    out[i] = '\0';
    return (int)i; // number of characters written (excluding '\0')
}

#define TEMP_BUFFER_LEN 64
static
char tempBuff[TEMP_BUFFER_LEN];

static const
char failAt[] = "Fail at \0";
#define RESERVE_AT_END 14
#define MAX_LINE_LEN (TEMP_BUFFER_LEN - RESERVE_AT_END - strlen(failAt))

uint32_t app_assert_prep(const uint8_t *file, uint32_t line)
{
    uint16_t len = 0, pos = 0, line_len = 0;

    strncpy(tempBuff, failAt, strlen(failAt));
    pos = strlen(failAt);

    len = strlen((const char *)file);
    if (len <= MAX_LINE_LEN) 
    {
        strncpy(&tempBuff[pos], (const char *)file, len);
    }
    else
    {
        strncpy(&tempBuff[pos], (const char *)file + 
                                (len - MAX_LINE_LEN), MAX_LINE_LEN);
        len = MAX_LINE_LEN;
    }
    pos += len++;
    len += strlen(failAt);
    tempBuff[pos++] = ':';
    line_len = u32_to_ascii((char *)&tempBuff[pos], 10, line);
    pos += line_len;
    len += line_len;
    tempBuff[pos++] = '\r';
    tempBuff[pos++] = '\n';
    tempBuff[pos++] = '\0';
    len += 3;

    return len;
}

static const
char prev_assert[] = "Previous assert error: \r\n\0";

void assert_storage_reset(void)
{
    uint32_t len;
    if (g_assert_info.magic != magic) return; // junk data in RAM

    if (! FLAG_HAS(g_assert_info.state, FLUSHED) &&
        ! FLAG_HAS(g_assert_info.state, UART_FAIL))
    {
        FDSendLog((const unsigned char *)prev_assert, strlen(prev_assert), 1, 0);
        
        len = app_assert_prep((const unsigned char *)g_assert_info.file, 
                              g_assert_info.line);
        FDSendLog((const unsigned char *)tempBuff, len, 1, 0);
    }
}

void app_assert_failed(uint8_t *file, uint32_t line)
{
    uint32_t len;
    
    g_assert_info.file = (const char *)file;
    g_assert_info.line = line;
    g_assert_info.magic = magic;
    FLAG_CLR(g_assert_info.state, FLUSHED);

    if (! FLAG_HAS(g_assert_info.state, UART_FAIL))
    {
        FLAG_SET(g_assert_info.state, UART_FAIL);
        len = app_assert_prep(file, line);
        FDSendLog((const unsigned char *)tempBuff, len, 1, 1);
    }
  
    FLAG_CLR(g_assert_info.state, UART_FAIL);
    FLAG_SET(g_assert_info.state, FLUSHED);

    __disable_irq();

#ifdef DEBUG
    __BKPT(0);
#endif

    while(1) {}
}

#endif
