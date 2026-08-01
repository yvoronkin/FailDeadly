#ifndef STM32_ASSERT_H
#define STM32_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef USE_FULL_ASSERT

void assert_storage_reset(void);
void assert_failed(uint8_t *file, uint32_t line);
void app_assert_failed(uint8_t *file, uint32_t line);

#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* STM32_ASSERT_H */
