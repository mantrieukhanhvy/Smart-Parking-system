#include "parking_config.h"

#define FLASH_BASE_ADDRESS 0x40023C00UL
#define PARKING_CONFIG_ADDRESS 0x080E0000UL
#define PARKING_CONFIG_MAGIC 0x5041524BUL
#define PARKING_CONFIG_CHECK 0xA57C39E1UL

#define FLASH_KEY1 0x45670123UL
#define FLASH_KEY2 0xCDEF89ABUL
#define FLASH_SR_BSY (1UL << 16)
#define FLASH_SR_EOP (1UL << 0)
#define FLASH_SR_ERRORS ((1UL << 1) | (1UL << 4) | (1UL << 5) | \
                         (1UL << 6) | (1UL << 7))
#define FLASH_SR_CLEAR (FLASH_SR_EOP | FLASH_SR_ERRORS)
#define FLASH_CR_PG (1UL << 0) // Program
#define FLASH_CR_SER (1UL << 1) // Sector Erase
#define FLASH_CR_STRT (1UL << 16) // Start
#define FLASH_CR_LOCK (1UL << 31) // Lock
#define FLASH_TIMEOUT 1000000UL

typedef struct {
    volatile uint32_t ACR; //access control register
    volatile uint32_t KEYR; //key register
    volatile uint32_t OPTKEYR; //option key register
    volatile uint32_t SR; //status register
    volatile uint32_t CR; //control register
} Flash_TypeDef;

#define FLASH ((Flash_TypeDef *)FLASH_BASE_ADDRESS)

typedef struct {
    uint32_t magic;
    uint32_t capacity;
    uint32_t check;
} ParkingConfigRecord;

static uint8_t Flash_WaitReady(void) {
    uint32_t timeout = FLASH_TIMEOUT;

    while ((FLASH->SR & FLASH_SR_BSY) && timeout > 0U) {
        timeout--;
    }

    return timeout > 0U;
}

static void Flash_Unlock(void) {
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

static uint8_t Flash_EraseConfigSector(void) {
    if (!Flash_WaitReady()) {
        return 0;
    }

    Flash_Unlock();
    if (FLASH->CR & FLASH_CR_LOCK) {
        return 0;
    }

    FLASH->SR = FLASH_SR_CLEAR;
    FLASH->CR &= ~((0xFUL << 3) | FLASH_CR_PG | FLASH_CR_SER |
                   (3UL << 8));
    FLASH->CR |= (11UL << 3) | FLASH_CR_SER | (2UL << 8);
    FLASH->CR |= FLASH_CR_STRT;

    if (!Flash_WaitReady() || (FLASH->SR & FLASH_SR_ERRORS)) {
        FLASH->CR &= ~FLASH_CR_SER;
        return 0;
    }

    FLASH->CR &= ~FLASH_CR_SER;
    return 1;
}

static uint8_t Flash_ProgramRecord(const ParkingConfigRecord *record) {
    volatile uint32_t *flash_word = (volatile uint32_t *)PARKING_CONFIG_ADDRESS;
    const uint32_t *record_word = (const uint32_t *)record;
    uint32_t i;

    if (!Flash_WaitReady()) {
        return 0;
    }

    Flash_Unlock();
    if (FLASH->CR & FLASH_CR_LOCK) {
        return 0;
    }

    FLASH->SR = FLASH_SR_CLEAR;
    FLASH->CR &= ~(FLASH_CR_SER | (3UL << 8));
    FLASH->CR |= FLASH_CR_PG | (2UL << 8);

    for (i = 0; i < sizeof(*record) / sizeof(uint32_t); i++) {
        flash_word[i] = record_word[i];
        if (!Flash_WaitReady() || (FLASH->SR & FLASH_SR_ERRORS) ||
            flash_word[i] != record_word[i]) {
            FLASH->CR &= ~FLASH_CR_PG;
            return 0;
        }
    }

    FLASH->CR &= ~FLASH_CR_PG;
    return 1;
}

static uint8_t ParkingConfig_IsValid(const ParkingConfigRecord *record) {
    return (record->magic == PARKING_CONFIG_MAGIC) &&
           (record->capacity == PARKING_DEFAULT_CAPACITY) &&
           (record->check ==
            (PARKING_CONFIG_MAGIC ^ record->capacity ^ PARKING_CONFIG_CHECK));
}

uint8_t ParkingConfig_Init(uint32_t *capacity) {
    const ParkingConfigRecord *stored =
        (const ParkingConfigRecord *)PARKING_CONFIG_ADDRESS;
    ParkingConfigRecord record;

    if (capacity == 0) {
        return 0;
    }

    if (ParkingConfig_IsValid(stored)) {
        *capacity = stored->capacity;
        return 1;
    }

    record.magic = PARKING_CONFIG_MAGIC;
    record.capacity = PARKING_DEFAULT_CAPACITY;
    record.check = PARKING_CONFIG_MAGIC ^ record.capacity ^ PARKING_CONFIG_CHECK;

    if (!Flash_EraseConfigSector() || !Flash_ProgramRecord(&record)) {
        FLASH->CR |= FLASH_CR_LOCK;
        return 0;
    }

    FLASH->CR |= FLASH_CR_LOCK;
    *capacity = record.capacity;
    return 1;
}
