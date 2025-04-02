# VR-PD-Stabilizing-Glove

# Hand Tremor Stabilization Glove

This project consists of three main components, each responsible for different aspects of the glove device, including microcontroller functionality, hardware design, and computer application development.

## Device Overview

Here is an image of the hand tremor stabilization glove:

![Glove Device](https://github.com/Peter-1119/VR-PD-Stabilizing-Glove/blob/main/Hardware_Design/Device%20Pics/515725_0.jpg)

The glove is designed to counteract tremors using an embedded motor and sensors to analyze and stabilize hand movements.

## Project Structure

```
📦 HandTremorGlove (Hand Tremor Stabilization Glove Project)
├── 📂 Firmware_STM32 (STM32 Firmware Development)
│   └── Glove (Glove Firmware Code)
├── 📂 Hardware_Design (Hardware Design)
│   ├── KiCad (Circuit Design)
│   ├── TinkerCad (Enclosure Design)
│   └── SolidWorks (Mechanical Design)
└── 📂 Software_Unity (Unity Software Development)
    └──  AirWrite (Virtual Reality Application)
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

Here is a demonstration of how the glove is visualized and functions in a virtual reality environment:

![VR Demo](https://github.com/Peter-1119/VR-PD-Stabilizing-Glove/blob/main/Software_Unity/AirWrite/Results/AirWrite-ezgif.com-video-to-gif-converter.gif)

The VR application allows users to see their hand movements in real-time, providing valuable feedback and interaction within a virtual space.

## Installation & Usage
For detailed installation and development instructions, please refer to the `README.md` files in each subdirectory.

## Contribution
If you are interested in contributing to this project, please submit a Pull Request or contact us.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
