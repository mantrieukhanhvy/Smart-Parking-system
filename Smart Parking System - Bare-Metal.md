# Smart Parking System - Bare-Metal STM32F407

## 1. Project Overview
This project is an automated smart parking model designed to operate intelligently and energy-efficiently. The system automates the entire parking workflow, from gate control via infrared sensing to individual slot status tracking via ultrasonic sensors. 

Currently, the project is in **Phase 1: Hardware bring-up**. It is being developed from scratch as an **Empty Project (Bare-metal C)** on the STM32CubeIDE without using the ST HAL (Hardware Abstraction Layer) libraries. All peripheral configurations are done by directly manipulating memory-mapped registers based on the RM0090 Reference Manual.

## 2. Hardware Architecture & Pinout
- **Microcontroller:** ARM Cortex-M4 (STM32F407VGT6 on STM32F407G-DISC1 board)
- **Clock Speed:** 16 MHz (Internal HSI)

### Peripheral Connections:
| Component | MCU Pin | Pin Mode | Description |
| :--- | :--- | :--- | :--- |
| **IR Obstacle Sensor** | `PE8` | GPIO Input | Detects vehicle presence at the entrance gate. |
| **HC-SR04 (Trigger)** | `PE9` | GPIO Output | Sends a 10µs HIGH pulse to initiate ultrasonic measurement. |
| **HC-SR04 (Echo)** | `PE10` | GPIO Input | Receives the acoustic reflection pulse. |
| **Servo Motor** | `PA6` | Alternate Function (AF2) | Receives 50Hz PWM signal to actuate the gate barrier. |

## 3. Software Architecture (Custom Drivers)
Instead of using standard libraries, a custom device header (`my_stm32f4.h`) was created to map the physical memory addresses to C structures for atomic bitwise operations.

### Implemented Modules:
*   **RCC (Reset and Clock Control):** 
    *   Enabled AHB1 bus clocks for `GPIOA` and `GPIOE`.
    *   Enabled APB1 bus clocks for `TIM2` and `TIM3`.
*   **GPIO Configuration:** 
    *   Configured `MODER` for Input, Output, and Alternate Function modes.
    *   Utilized the `BSRR` register for fast, atomic pin toggling (essential for the 10µs HC-SR04 trigger).
    *   Configured `AFRL` to route `TIM3_CH1` to pin `PA6`.
*   **TIM2 (Hardware Microsecond Timer):**
    *   Prescaler configured to divide the 16MHz clock to 1MHz (1µs per tick).
    *   Used to measure the exact HIGH pulse duration of the HC-SR04 Echo pin to calculate distance accurately in centimeters.
    *   Also acts as a precise delay generator (replacing blocking `for`-loop delays) to prevent acoustic interference.
*   **TIM3 (PWM Generation):**
    *   Configured to generate a 50Hz (20ms period) PWM signal.
    *   Output Compare Mode (PWM Mode 1) enabled on Channel 1.
    *   The `CCR1` register is dynamically updated to control the servo motor angle (1000µs for 0° closed, 1500µs for 90° open).

## 4. Current System Workflow
1. The system continually polls the state of the IR sensor at pin `PE8`.
2. When a vehicle breaks the infrared beam (pin goes LOW), the MCU sets `TIM3->CCR1` to 1500, opening the servo barrier automatically.
3. When the vehicle leaves (pin goes HIGH), the barrier is closed (`TIM3->CCR1` = 1000).
4. Simultaneously, the `HCSR04_GetDistance()` function is called periodically to monitor the parking slot's occupancy status using `TIM2`.

### Current Occupancy and Capacity Behavior
The HC-SR04 measures the slot continuously. A valid reading at or below 5 cm marks it occupied; a larger reading marks it vacant. The LCD shows `Free: vacancy/capacity`. The entrance IR sensor at `PE8` is only read while at least one space is vacant; the gate stays closed if the slot is occupied or the ultrasonic measurement times out.

The current build supports one monitored space. To monitor multiple separate spaces, add one HC-SR04 per space and extend the GPIO/sensor configuration; one fixed sensor cannot determine occupancy of several separate spaces. Capacity defaults to one and is initialized in the final STM32F407 Flash sector, which is reserved from firmware placement by the linker. Vacancy is recalculated from the sensor after every boot and is not written to Flash.

## 5. Next Steps (Roadmap)
- **Phase 1 (Remaining):** Integrate I2C communication by configuring `I2C1` registers to drive a local LCD display for status metrics.
- **Phase 2:** Integrate the **FreeRTOS** kernel. Port the bare-metal polling logic into preemptive concurrent tasks (`vSensorTask`, `vGateTask`, `vDisplayTask`) utilizing Queues and Mutexes.
- **Phase 3:** Implement Power Management. Utilize External Interrupts (EXTI) on the IR sensor to wake the MCU from low-power Stop Mode.