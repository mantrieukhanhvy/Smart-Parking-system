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
#include "lcd_i2c.h"
#include "parking_config.h"
#include "led.h"
#include "semphr.h" // for mutex

/* --------------------------------------------------------------------------
 * Application tuning
 * ------------------------------------------------------------------------ */
#define PARKING_OCCUPIED_DISTANCE_CM 5.0f

/* Shared parking capacity initialized in main */
extern uint32_t parking_capacity;

/* Shared global state variables */
static uint32_t global_vacancy = 0;
static uint8_t global_sensor_ok = 0;
static uint8_t global_car_at_gate = 0;

/* Mutex to protect shared state between tasks */
static SemaphoreHandle_t xStateMutex = NULL;

/**
 * @brief High-priority task for sensors, LEDs, and gate servo control.
 */
static void vSensorGateTask(void *pvParameters) {
    float distance_cm;
    uint8_t occupied = 0;

    for (;;) {
        uint8_t sensor_ok = HCSR04_TryGetDistance(&distance_cm);

        if (sensor_ok) {
            occupied = (distance_cm <= PARKING_OCCUPIED_DISTANCE_CM);
            LED_SetParkingStatus(occupied ? LED_STATUS_OCCUPIED : LED_STATUS_VACANT);
        } else {
            LED_SetParkingStatus(LED_STATUS_UNKNOWN);
        }

        uint32_t vacancy = sensor_ok ? parking_capacity - occupied : 0U;
        uint8_t car_at_gate = 0;

        if (vacancy > 0U) {
            car_at_gate = IR_Sensor_Read();
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
    uint32_t local_vacancy;
    uint8_t local_sensor_ok;
    uint8_t local_car_at_gate;

    for (;;) {
        // Safely copy shared state variables
        if (xSemaphoreTake(xStateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            local_vacancy = global_vacancy;
            local_sensor_ok = global_sensor_ok;
            local_car_at_gate = global_car_at_gate;
            xSemaphoreGive(xStateMutex);
        }

        LCD_SetCursor(0, 0);
        LCD_Print("Free: ");
        LCD_PrintInt((int)local_vacancy);
        LCD_Print("/");
        LCD_PrintInt((int)parking_capacity);
        LCD_Print("       ");

        LCD_SetCursor(1, 0);
        if (!local_sensor_ok) {
            LCD_Print("Sensor: ERROR   ");
        } else {
            LCD_Print("Gate: ");
            LCD_Print(local_car_at_gate ? "OPEN   " : "CLOSED ");
        }
        
        // Slow, non-critical LCD updates (e.g., every 400ms)
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
        configMINIMAL_STACK_SIZE * 2,   /* Stack depth */
        NULL,                           /* Task parameters */
        tskIDLE_PRIORITY + 1,           /* Low Task priority */
        NULL                            /* Task handle */
    );
}