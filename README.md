# Light Sensor Embedded System

This repository contains the firmware and documentation for a light-sensing embedded system built around an STM32 microcontroller and a custom PCB. The project originally began as a school embedded systems project and has since been transferred to this personal repository for continued development, documentation, and refinement.

The system is designed to measure light intensity using a light-to-frequency sensor, process the signal on an STM32 microcontroller, and support serial communication, timestamping, and data logging for future environmental monitoring use cases.

## Project Status

This project is currently being continued as a personal embedded systems project. The original prototype was developed using an STM32 Nucleo development board, and the current version is being adapted to run on a custom PCB.

Current focus areas include:

- Migrating the firmware from the development board setup to the custom PCB
- Verifying UART communication on the custom hardware
- Testing sensor measurements from the PCB-mounted light sensor
- Improving flash logging and timestamped data storage
- Preparing the system for a more complete standalone prototype

## Project Overview

The goal of this project is to build a compact embedded light sensor capable of collecting and storing light measurements. The system uses a light-to-frequency sensor whose output frequency changes based on the amount of detected light. The STM32 measures this frequency using timer input capture and can communicate with a user through a UART command interface.

This type of system could be used for environmental monitoring, light pollution studies, outdoor sensing, or as a foundation for a low-power remote sensor node.

## Features

- STM32-based embedded firmware
- Custom PCB-based hardware design
- Light-to-frequency sensor integration
- Timer input capture for measuring sensor output
- UART serial command interface
- Interrupt-driven UART receive handling
- Circular buffer for incoming commands
- Real-time clock support for timestamps
- Flash memory logging for persistent records
- Command parsing for setting and reading system values
- Prototype-focused hardware and firmware documentation

## Hardware

The current hardware design uses a custom PCB instead of only relying on the STM32 Nucleo development board. The PCB is designed to support the core sensing, communication, and power components needed for the light sensor prototype.

### Main Hardware Components

| Component | Purpose |
|---|---|
| STM32 microcontroller | Main embedded controller |
| TSL237 light-to-frequency sensor | Measures light through frequency output |
| Custom PCB | Integrates the sensor, controller, power, and communication hardware |
| UART connector | Serial communication and debugging |
| RTC support | Provides time and date information for logged records |
| Battery holder / power input | Supports standalone prototype development |
| Oscillator / clock components | Supports timing requirements |
| Debug/programming interface | Allows firmware flashing and debugging |

## Software / Firmware Stack

- **Language:** Embedded C
- **IDE:** STM32CubeIDE
- **Configuration Tool:** STM32CubeMX
- **Platform:** STM32
- **Communication:** UART
- **Key Peripherals:**
  - GPIO
  - UART / USART
  - Timers
  - RTC
  - Flash memory
  - Interrupts

## System Architecture

```text
Light Source
    |
    v
TSL237 Light-to-Frequency Sensor
    |
    v
STM32 Timer Input Capture
    |
    v
Embedded Firmware
    |
    +--> UART Command Interface
    |
    +--> RTC Timestamping
    |
    +--> Flash Data Logging
    |
    v
Stored or Transmitted Sensor Records
