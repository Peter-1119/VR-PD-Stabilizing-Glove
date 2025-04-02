# Parkinson's Disease Hand Tremor Analysis and Stabilization Device - Firmware

## Project Description

This project focuses on the firmware development for a device aimed at analyzing and stabilizing hand tremors in Parkinson's disease patients. It involves developing the computational functions of the STM32, including FFT-based tremor detection and attitude calculation.

## Key Features

*   **STM32 Computational Function Development**:
    *   Development of code for inertial measurement unit (IMU) data acquisition.
    *   Implementation of the FFT (Fast Fourier Transform) algorithm for tremor frequency analysis.
    *   Implementation of attitude calculation algorithms for hand attitude tracking.
    *   Development of gyroscope control code to provide physical stabilization.
    *   Development of communication code (e.g., Bluetooth) for data transmission.

## Technical Details

*   **Microcontroller**: STM32 (or other suitable model)
*   **Development Environment**: (e.g., STM32CubeIDE)
*   **Programming Language**: C/C++
*   **FFT Library**: (e.g., CMSIS-DSP)
*   **Attitude Calculation**: (e.g., Quaternion algorithms)
*   **Communication Protocol**: Bluetooth and USB

## Directory Structure

```
/Glove
├── Core
│   ├── Src                 # Source Files
│   │   ├── main.c          # Main Application
│   │   ├── ICM20948_WE.c   # IMU Driver
│   │   ├── IMU_task.c      # IMU Data Processing & Tremor Detection Algorithm
│   │   └── mahony.c        # Attitude Calculation (e.g., Mahony Filter Implementation)
│   └── inc/                # Header Files
└── STM32_WPAN/App          # Bluetooth Low Energy (BLE) Files
    └── custom_app.c        # BLE Application Logic
```

