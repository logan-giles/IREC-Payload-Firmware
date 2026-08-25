# IREC Payload Firmware

Firmware for an embedded aerospace payload developed for the Mizzou Space
Program's Intercollegiate Rocket Engineering Competition (IREC) vehicle.

The system is built around the XIAO ESP32-S3 PLUS and uses ESP-IDF and FreeRTOS
to interface with multiple sensors and manage payload data acquisition.

## Current Status

This project is actively under development.

Current work includes:
- ESP32-S3 firmware architecture using FreeRTOS
- Custom hardware abstraction wrappers around sensor libraries
- Initial sensor interface development
- System-level firmware structure
- Integration with a custom avionics carrier PCB

Current GPIO assignments and peripheral configurations were established
for breadboard-based prototyping and will change during final PCB integration.

## Hardware

- XIAO ESP32-S3 Plus
- BNO085 IMU
- BMP390 barometric pressure sensor
- BME688 environmental sensor
- microSD storage
- Custom avionics carrier PCB

The custom PCB provides power distribution, sensor interfaces, and
connections between the ESP32-S3 and payload electronics.

## Firmware Architecture

The firmware uses custom hardware abstraction layers around individual
sensor libraries. This keeps application code independent of
component-specific implementations and simplifies future sensor
replacement.

The current firmware architecture is being developed around FreeRTOS
tasks for sensor acquisition, processing, and data handling.

## Development

- ESP-IDF
- FreeRTOS
- C / Embedded C
