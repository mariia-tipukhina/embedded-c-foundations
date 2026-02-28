# Embedded C & IoT Security Foundations

A repository tracking my progression in bare-metal STM32 firmware development, peripheral driver implementation, and hardware-level security.

## Hardware Used
* MCU: STM32F103C8T6 (Blue Pill)
* Programmer: ST-Link V2
* Communication: USB-to-TTL Serial Converter

## Project Milestones

### 1. Bare-Metal Memory & Pointer Manipulation
* Bypassed standard HAL libraries to interact directly with hardware registers.
* Configured RCC (Reset and Clock Control) and GPIO memory-mapped registers to drive external peripherals.

### 2. Serial Communication Interface (UART)
* Developed a bi-directional UART driver operating at 9600 baud.
* Calculated precise baud rate dividers based on the 8MHz system clock.

### 3. Buffer Overflow Protection & Command Parsing
* Implemented a secure UART command line interface.
* Engineered buffer overflow protection for the RX character array, ensuring memory integrity during continuous serial data stream reception (IoT Security foundation).

## Development Environment
* OS: Ubuntu (WSL2)
* Toolchain: GCC ARM Embedded
* IDE: STM32CubeIDE (for hardware debugging)