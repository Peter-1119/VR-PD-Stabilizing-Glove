# VR-PD-Stabilizing-Glove

# Hand Tremor Stabilization Glove

This project consists of three main components, each responsible for different aspects of the glove device, including microcontroller functionality, hardware design, and computer application development.

## Project Structure

```
📦 HandTremorGlove
├── 📂 Glove_Function/STM32
│   ├── Core/Src/*.c                   -- Fundamental GPIO functions (timer, PWM, I2C) and algorithms (Mahony Filter, FFT)
│   └── STM32_WPAN/App/custom_app.c    -- Motor control and message transmission via USB and BLE
│
├── 📂 Hardware_Design
│   ├── Glove_Board/                   -- PCB layout for the glove board
│   ├── InertialSensor/                -- PCB layout for the sensor module
│   ├── Shells/                        -- Housing for the main control circuit and motor
│   └── Flywheel/                      -- 3D model of the flywheel
│
└── 📂 VR_Unity_Application/AirWrite/Assets
    ├── BasicFunc/*.cs                 -- Hand finger node calculations and global storage
    ├── Scenes/*                       -- Scene functionalities and fundamental settings
    └── ./*.cs                         -- Event handling functions for each scene
```

## 1. Glove_Function
This module manages the STM32 microcontroller for the glove device, including:
- Processing sensor data within the glove
- Controlling the motor and stabilizing hand tremors
- Implementing communication protocols (I2C)
- Developing firmware and optimizing algorithms

## 2. Hardware_Design
This module focuses on the electronic and mechanical design of the glove, including:
- Circuit design and PCB layout
- 3D modeling and mechanical development
- Power management and port configurations

## 3. VR_Unity_Application
This module develops a VR application for rendering the glove device and tracking hand posture, including:
- Hand gesture tracking and posture detection
- 3D model rendering and interaction
- VR integration and application development

## Installation & Usage
For detailed installation and development instructions, please refer to the `README.md` files in each subdirectory.

## Contribution
If you are interested in contributing to this project, please submit a Pull Request or contact us.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
