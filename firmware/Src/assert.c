#include <stdint.h>
#include <string.h>

#include "stm32_assert.h"
#include "main.h"

#include "fd_log.h"

#ifdef USE_FULL_ASSERT

typedef
struct {
    const char * file;
    uint32_t line;
    int uart_fail;
} assert_info_t;

volatile assert_info_t g_assert_info;

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

void assert_reset()
{
    g_assert_info.uart_fail = 0;
}

#define TEMP_BUFFER_LEN 64
static
char tempBuff[TEMP_BUFFER_LEN];

static const
char failAt[] = "Fail at \0";
#define RESERVE_AT_END 14
#define MAX_LINE_LEN (TEMP_BUFFER_LEN - RESERVE_AT_END - strlen(failAt))

void assert_failed(uint8_t *file, uint32_t line)
{ 
    uint16_t len = 0, pos = 0, line_len = 0;

    g_assert_info.file = (const char *)file;
    g_assert_info.line = line;

    if (g_assert_info.uart_fail == 0)
    {
        g_assert_info.uart_fail = 1;
        
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

        FDSendLog((const unsigned char *)tempBuff, len, 1, 1);
    }
    
    g_assert_info.uart_fail = 0;

    __disable_irq();

#ifdef DEBUG
    __BKPT(0);
#endif

    while(1) {}
}

#endif
