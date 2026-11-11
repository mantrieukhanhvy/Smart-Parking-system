#include "retarget.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#define DEMCR       (*(volatile uint32_t *)0xE000EDFCUL)
#define TPI_ACPR    (*(volatile uint32_t *)0xE0040010UL)
#define TPI_SPPR    (*(volatile uint32_t *)0xE00400F0UL)
#define TPI_FFCR    (*(volatile uint32_t *)0xE0040304UL)
#define ITM_STIM0   (*(volatile uint32_t *)0xE0000000UL)
#define ITM_TER     (*(volatile uint32_t *)0xE0000E00UL)
#define ITM_TCR     (*(volatile uint32_t *)0xE0000E80UL)
#define ITM_LAR     (*(volatile uint32_t *)0xE0000FB0UL)

#define DEMCR_TRCENA    (1UL << 24)
#define ITM_TCR_ITMENA  (1UL << 0)
#define ITM_TCR_TSENA   (1UL << 1)
#define ITM_TCR_SYNCENA (1UL << 2)
#define SWO_CPU_CLOCK_HZ 16000000UL
#define SWO_BAUD_RATE    2000000UL

void SWO_Init(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    DEMCR |= DEMCR_TRCENA;
    TPI_ACPR = (SWO_CPU_CLOCK_HZ / SWO_BAUD_RATE) - 1UL;
    TPI_SPPR = 2UL;
    TPI_FFCR = 0x100UL;

    ITM_LAR = 0xC5ACCE55UL;
    ITM_TCR = ITM_TCR_ITMENA | ITM_TCR_SYNCENA | ITM_TCR_TSENA;
    ITM_TER = 1UL;
}

int _write(int file, char *ptr, int len)
{
    (void)file;

    if ((ITM_TCR & ITM_TCR_ITMENA) == 0U ||
        (ITM_TER & 1UL) == 0U) {
        errno = EIO;
        return -1;
    }

    for (int i = 0; i < len; i++) {
        while ((ITM_STIM0 & 1UL) == 0U) {
        }
        ITM_STIM0 = (uint8_t)ptr[i];
    }
    return len;
}
