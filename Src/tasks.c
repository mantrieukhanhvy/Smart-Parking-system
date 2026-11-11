/**
 ******************************************************************************
 * @file    tasks.c
 * @brief   FreeRTOS task layer for the smart parking system.
 ******************************************************************************
 */
#include "tasks.h"

#include "FreeRTOS.h"
#include "task.h"
#include "ir_sensor.h"
#include "hcsr04.h"
#include "servo.h"
#include "parking_config.h"
#include "led.h"
#include "semphr.h" // for mutex
#include <stdint.h>
#include <stdio.h>
/* --------------------------------------------------------------------------
 * Application tuning
 * ------------------------------------------------------------------------ */
#define PARKING_OCCUPIED_DISTANCE_CM 5.0f
#define ULTRASONIC_CONFIRM_COUNT 3U
#define IR_CONFIRM_MS 500U

/* Shared global state variables */
static uint32_t global_vacancy = 0;
static uint8_t global_sensor_ok = 0;
static uint8_t global_car_at_gate = 0;

/* Mutex to protect shared state between tasks */
static SemaphoreHandle_t xStateMutex = NULL;

static uint8_t IR_Sensor_ReadStable(void)
{
    static uint8_t stable_state = 0;
    static uint8_t candidate_state = 0;
    static TickType_t candidate_since = 0;
    const uint8_t raw_state = IR_Sensor_Read();
    const TickType_t now = xTaskGetTickCount();

    if (raw_state != candidate_state) {
        candidate_state = raw_state;
        candidate_since = now;
    }

    if (candidate_state != stable_state &&
        (TickType_t)(now - candidate_since) >= pdMS_TO_TICKS(IR_CONFIRM_MS)) {
        stable_state = candidate_state;
    }

    return stable_state;
}

static void UpdateOccupancy(uint8_t sensor_ok,
                            float distance_cm,
                            uint8_t *occupied)
{
    static uint8_t candidate_occupied = 0;
    static uint8_t consecutive_count = 0;
    uint8_t reading_occupied;

    if (!sensor_ok) {
        consecutive_count = 0;
        return;
    }

    reading_occupied = distance_cm <= PARKING_OCCUPIED_DISTANCE_CM;
    if (reading_occupied == *occupied) {
        consecutive_count = 0;
        return;
    }

    if (reading_occupied != candidate_occupied) {
        candidate_occupied = reading_occupied;
        consecutive_count = 1U;
    } else if (consecutive_count < ULTRASONIC_CONFIRM_COUNT) {
        consecutive_count++;
    }

    if (consecutive_count >= ULTRASONIC_CONFIRM_COUNT) {
        *occupied = candidate_occupied;
        consecutive_count = 0;
    }
}

/**
 * @brief High-priority task for sensors, LEDs, and gate servo control.
 */
static void vSensorGateTask(void *pvParameters) {
    float distance_cm;
    uint8_t occupied = 0;

    for (;;) {
        uint8_t sensor_ok = HCSR04_TryGetDistance(&distance_cm);

        UpdateOccupancy(sensor_ok, distance_cm, &occupied);

        if (sensor_ok) {
            LED_SetParkingStatus(occupied ? LED_STATUS_OCCUPIED : LED_STATUS_VACANT);
        } else {
            LED_SetParkingStatus(LED_STATUS_UNKNOWN);
        }

        uint32_t vacancy = sensor_ok ? parking_capacity - occupied : 0U;
        uint8_t car_at_gate = 0;
        uint8_t stable_car_at_gate = IR_Sensor_ReadStable();

        uint32_t pe8_level = (GPIOE->IDR >> 8U) & 1U;

        printf("PE8=%lu | IR raw=%u | IR stable=%u | ultrasonic OK=%u | vacancy=%lu\r\n",
               (unsigned long)pe8_level,
               (unsigned int)(pe8_level == 0U),
               (unsigned int)stable_car_at_gate,
               (unsigned int)sensor_ok,
               (unsigned long)vacancy);

        if (vacancy > 0U) {
            car_at_gate = stable_car_at_gate;
        }

        Servo_SetAngle(car_at_gate);

        // Safely update shared state variables for the display task
        if (xSemaphoreTake(xStateMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            global_vacancy = vacancy;
            global_sensor_ok = sensor_ok;
            global_car_at_gate = car_at_gate;
            xSemaphoreGive(xStateMutex);
        }

        // Run this check relatively frequently (e.g., every 100ms)
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief Low-priority task dedicated to updating the I2C LCD.
 */
static void vDisplayTask(void *pvParameters) {
    uint32_t local_vacancy = 0;
    uint8_t local_sensor_ok = 0;
    uint8_t local_car_at_gate = 0;

    for (;;) {
        // Safely copy shared state variables
        if (xSemaphoreTake(xStateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            local_vacancy = global_vacancy;
            local_sensor_ok = global_sensor_ok;
            local_car_at_gate = global_car_at_gate;
            xSemaphoreGive(xStateMutex);
        }

        // Print formatted status line to the console
        if (!local_sensor_ok) {
            printf("Sensor: ERROR\r\n");
        } else {
            printf("Free: %lu/%lu | Gate: %s\r\n",
                   (unsigned long)local_vacancy,
                   (unsigned long)parking_capacity,
                   local_car_at_gate ? "OPEN" : "CLOSED");
        }
        
        // Slow, non-critical status updates (e.g., every 400ms)
        vTaskDelay(pdMS_TO_TICKS(400));
    }
}

void Create_Parking_Tasks(void) {
    // Create the mutex for thread-safe data sharing
    xStateMutex = xSemaphoreCreateMutex();
    configASSERT(xStateMutex != NULL);

    // Create High-Priority Sensor & Gate Task
    xTaskCreate(
        vSensorGateTask,
        "SensorGateTask",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        tskIDLE_PRIORITY + 2,           // High Task priority
        NULL
    );

    // Create Low-Priority Display Task
    xTaskCreate(
        vDisplayTask,                   /* Task function */
        "DisplayTask",                  /* Task name for debugging */
        configMINIMAL_STACK_SIZE * 4,   /* Stack depth */
        NULL,                           /* Task parameters */
        tskIDLE_PRIORITY + 1,           /* Low Task priority */
        NULL                            /* Task handle */
    );
}
