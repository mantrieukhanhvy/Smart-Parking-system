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

/* --------------------------------------------------------------------------
 * Application tuning
 * ------------------------------------------------------------------------ */
#define PARKING_OCCUPIED_DISTANCE_CM   5.0f

/* Shared parking capacity initialized in main */
extern uint32_t parking_capacity;

/**
 * @brief Main parking system task loop.
 */

    static void vParkingTask(void *pvParameters) {
    uint32_t vacancy;
    uint8_t sensor_ok;
    uint8_t occupied = 0;
    uint8_t car_at_gate;
    float distance_cm;

    for (;;) {
        sensor_ok = HCSR04_TryGetDistance(&distance_cm);

        if (sensor_ok) {
            occupied = (distance_cm <= PARKING_OCCUPIED_DISTANCE_CM);
            LED_SetParkingStatus(occupied ? LED_STATUS_OCCUPIED : LED_STATUS_VACANT);
        } else {
            LED_SetParkingStatus(LED_STATUS_UNKNOWN);
        }

        vacancy = sensor_ok ? parking_capacity - occupied : 0U;
        car_at_gate = 0;

        if (vacancy > 0U) {
            car_at_gate = IR_Sensor_Read();
        }

        Servo_SetAngle(car_at_gate);

        LCD_SetCursor(0, 0);
        LCD_Print("Free: ");
        LCD_PrintInt((int)vacancy);
        LCD_Print("/");
        LCD_PrintInt((int)parking_capacity);
        LCD_Print("       ");

        LCD_SetCursor(1, 0);
        if (!sensor_ok) {
            LCD_Print("Sensor: ERROR   ");
        } else {
            LCD_Print("Gate: ");
            LCD_Print(car_at_gate ? "OPEN   " : "CLOSED ");
        }

        HCSR04_Delay(); 
    }
}

void Create_Parking_Tasks(void) {
    xTaskCreate(
        vParkingTask,                   /* Task function */
        "ParkingTask",                  /* Task name for debugging */
        configMINIMAL_STACK_SIZE * 2,   /* Stack depth */
        NULL,                           /* Task parameters */
        tskIDLE_PRIORITY + 1,           /* Task priority */
        NULL                            /* Task handle */
    );
}