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

#define STATE_LOCK_TIMEOUT_MS 50U

extern uint32_t parking_capacity;

typedef struct {
    uint8_t sensor_ok;
    uint8_t occupied;
    uint8_t car_at_gate;
    uint32_t vacancy;
    uint8_t gate_open;
    uint8_t led_status;
    float distance_cm;
} parking_state_t;

static parking_state_t global_state;
static SemaphoreHandle_t xStateMutex = NULL;

static void vParkingLogicTask(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        if (xSemaphoreTake(xStateMutex, pdMS_TO_TICKS(STATE_LOCK_TIMEOUT_MS)) == pdTRUE) {
            uint8_t sensor_ok = global_state.sensor_ok;
            uint8_t occupied = global_state.occupied;
            uint8_t car_at_gate = global_state.car_at_gate;
            uint32_t vacancy = 0U;
            uint8_t gate_open;
            uint8_t led_status;

            if (sensor_ok && (occupied < parking_capacity)) {
                vacancy = parking_capacity - occupied;
            }

            gate_open = (vacancy > 0U) ? car_at_gate : 0U;
            led_status = !sensor_ok ? LED_STATUS_UNKNOWN :
                         (occupied ? LED_STATUS_OCCUPIED : LED_STATUS_VACANT);

            global_state.vacancy = vacancy;
            global_state.gate_open = gate_open;
            global_state.led_status = led_status;

            xSemaphoreGive(xStateMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void vIRTask(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        uint8_t car_at_gate = IR_Sensor_Read();

        if (xSemaphoreTake(xStateMutex, pdMS_TO_TICKS(STATE_LOCK_TIMEOUT_MS)) == pdTRUE) {
            global_state.car_at_gate = car_at_gate;
            xSemaphoreGive(xStateMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void vUltrasonicTask(void *pvParameters) {
    float distance_cm = 0.0f;

    (void)pvParameters;

    for (;;) {
        uint8_t sensor_ok = HCSR04_TryGetDistance(&distance_cm);
        uint8_t occupied = sensor_ok &&
            (distance_cm <= PARKING_OCCUPIED_DISTANCE_CM);

        if (xSemaphoreTake(xStateMutex, pdMS_TO_TICKS(STATE_LOCK_TIMEOUT_MS)) == pdTRUE) {
            global_state.sensor_ok = sensor_ok;
            global_state.occupied = occupied;
            global_state.distance_cm = distance_cm;
            xSemaphoreGive(xStateMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void vServoTask(void *pvParameters) {
    uint8_t last_gate_open = 0U;

    (void)pvParameters;

    for (;;) {
        uint8_t gate_open = last_gate_open;

        if (xSemaphoreTake(xStateMutex, pdMS_TO_TICKS(STATE_LOCK_TIMEOUT_MS)) == pdTRUE) {
            gate_open = global_state.gate_open;
            xSemaphoreGive(xStateMutex);
        }

        if (gate_open != last_gate_open) {
            Servo_SetAngle(gate_open);
            last_gate_open = gate_open;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void vLEDTask(void *pvParameters) {
    uint8_t last_status = LED_STATUS_UNKNOWN;

    (void)pvParameters;

    for (;;) {
        uint8_t status = last_status;

        if (xSemaphoreTake(xStateMutex, pdMS_TO_TICKS(STATE_LOCK_TIMEOUT_MS)) == pdTRUE) {
            status = global_state.led_status;
            xSemaphoreGive(xStateMutex);
        }

        if (status != last_status) {
            LED_SetParkingStatus(status);
            last_status = status;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void vDisplayTask(void *pvParameters) {
    uint32_t local_vacancy = 0U;
    uint8_t local_sensor_ok = 0U;
    uint8_t local_gate_open = 0U;

    (void)pvParameters;

    for (;;) {
        if (xSemaphoreTake(xStateMutex, pdMS_TO_TICKS(STATE_LOCK_TIMEOUT_MS)) == pdTRUE) {
            local_vacancy = global_state.vacancy;
            local_sensor_ok = global_state.sensor_ok;
            local_gate_open = global_state.gate_open;
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
            LCD_Print(local_gate_open ? "OPEN   " : "CLOSED ");
        }

        vTaskDelay(pdMS_TO_TICKS(400));
    }
}

void Create_Parking_Tasks(void) {
    global_state.sensor_ok = 0U;
    global_state.occupied = 0U;
    global_state.car_at_gate = 0U;
    global_state.vacancy = 0U;
    global_state.gate_open = 0U;
    global_state.led_status = LED_STATUS_UNKNOWN;
    global_state.distance_cm = 0.0f;

    xStateMutex = xSemaphoreCreateMutex();
    configASSERT(xStateMutex != NULL);

    configASSERT(xTaskCreate(vParkingLogicTask, "ParkingLogic",
        configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 5, NULL) == pdPASS);

    configASSERT(xTaskCreate(vIRTask, "IRSensor",
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL) == pdPASS);

    configASSERT(xTaskCreate(vUltrasonicTask, "Ultrasonic",
        configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 3, NULL) == pdPASS);

    configASSERT(xTaskCreate(vServoTask, "Servo",
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);

    configASSERT(xTaskCreate(vLEDTask, "LED",
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS);

    configASSERT(xTaskCreate(vDisplayTask, "Display",
        configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS);
}
