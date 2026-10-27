# Smart Parking System - Bare-Metal STM32F407

## 1. Project Overview
This project is an automated smart parking prototype built around an STM32F407VGT6. It controls an entrance gate, monitors parking-space occupancy, and displays vacancy on an I2C LCD.

The project is in **Phase 1: Hardware bring-up**. It is written in bare-metal C without the ST HAL; peripheral configuration is performed by directly manipulating memory-mapped registers based on the RM0090 Reference Manual.

## 2. Hardware Architecture & Pinout
- **Microcontroller:** ARM Cortex-M4 (STM32F407VGT6 on STM32F407G-DISC1 board)
- **Clock Speed:** 16 MHz (Internal HSI)

### Peripheral Connections
| Component | MCU Pin | Pin Mode | Description |
| :--- | :--- | :--- | :--- |
| **IR Obstacle Sensor** | `PE8` | GPIO Input | Detects a vehicle at the entrance; active-low. |
| **HC-SR04 Trigger** | `PE9` | GPIO Output | Sends a 10 us pulse to start a distance measurement. |
| **HC-SR04 Echo** | `PE10` | GPIO Input | Receives the ultrasonic echo pulse. |
| **Servo Motor** | `PA6` | Alternate Function (AF2) | Receives 50 Hz PWM to actuate the gate. |
| **Green LED** | `PB0` | GPIO Output | On when a valid reading indicates the space is vacant. |
| **Red LED** | `PB1` | GPIO Output | On when a valid reading indicates the space is occupied. |
| **I2C LCD** | `PB6` / `PB7` | Alternate Function (AF4) | Displays vacancy and gate or sensor status. |

## 3. Software Architecture
The custom device header (`my_stm32f4.h`) maps peripheral memory addresses to C structures for register access.

### Implemented Modules
* **RCC and GPIO:** Enables clocks for GPIOA, GPIOB, GPIOE, TIM2, TIM3, and I2C1. Configures sensor inputs, LED outputs, the servo alternate function, and I2C pins.
* **TIM2:** Configured for a 1 MHz counter to time HC-SR04 echo pulses and provide delays.
* **HC-SR04 driver:** Sends the trigger pulse, measures the echo with a timeout, and returns a distance in centimeters when the measurement succeeds.
* **TIM3 servo PWM:** Generates a 50 Hz signal on `PA6`; pulse widths control the closed and open gate positions.
* **I2C LCD driver:** Drives the local character LCD and prints text and integer values.
* **LED driver:** Controls the green vacant and red occupied indicators on `PB0` and `PB1`. Both LEDs are off when the distance reading is invalid.
* **Parking configuration:** Initializes the capacity (currently one space) in the final STM32F407 Flash sector. The linker reserves this sector from firmware placement. Vacancy is re-detected after reset rather than stored in Flash.

## 4. Current System Workflow
1. The main loop triggers the HC-SR04 and measures the echo pulse using TIM2.
2. A valid distance of `5 cm` or less marks the monitored space occupied: the red LED turns on and the green LED turns off. A larger valid distance marks it vacant and turns on the green LED. If no valid echo is received, both LEDs turn off and the entrance remains closed.
3. The LCD displays available spaces as `Free: vacancy/capacity`. It also displays gate status or a sensor error.
4. The entrance IR sensor on `PE8` is read only when the sensor reports at least one vacant space. While a car is detected, the servo opens the gate; otherwise it stays closed. If the space is occupied or the ultrasonic measurement fails, the gate remains closed.
5. Capacity is currently one. Vacancy is recalculated from the sensor after each reset and is not written to Flash.

The current hardware monitors one space. Monitoring multiple separate spaces requires one HC-SR04 per space and additional sensor wiring and configuration; one fixed sensor cannot determine occupancy in several separate spaces.

## 5. Next Steps
- **Phase 1:** Validate the 5 cm threshold and hardware behavior on the parking model.
- **Phase 2:** Integrate the FreeRTOS kernel and port polling logic into concurrent sensor, gate, and display tasks.
- **Phase 3:** Implement power management using an external interrupt on the IR sensor to wake the MCU from low-power Stop mode.
