# Project Abstract

## QNX Autonomous Vehicle Control Simulator

Autonomous vehicle systems require reliable communication between sensing, decision-making, control, and actuation components. This project presents a software-in-the-loop simulation of a simplified autonomous vehicle control pipeline using the QNX Neutrino Real-Time Operating System.

The system is divided into independent processes for sensor simulation, planning, control, and actuation. The sensor simulator generates parameters such as obstacle distance, lane offset, and vehicle speed. These values are transmitted to the planner using QNX native inter-process communication.

Based on the received sensor information, the planner determines an appropriate vehicle action such as `BRAKE`, `STEER_CORRECT`, or `MAINTAIN`. The controller then converts this decision into throttle and steering commands using proportional control logic.

The actuator receives the resulting control commands and communicates the vehicle state to a host machine through TCP sockets. A Python-based dashboard provides a visual representation of the received throttle and steering information.

The project demonstrates the use of QNX process architecture, synchronous message passing, embedded control logic, TCP communication, and host-side telemetry visualization in an autonomous vehicle simulation environment.

## Key Objectives

- Demonstrate process-to-process communication using QNX native IPC.
- Model a simplified autonomous vehicle sensing and control pipeline.
- Implement basic decision-making based on simulated sensor inputs.
- Convert high-level driving decisions into low-level control commands.
- Transmit actuator telemetry to a host-based visualization application.
- Gain practical exposure to QNX Neutrino and real-time embedded system concepts.

## Technologies Used

- QNX Neutrino RTOS
- C
- QNX Native IPC
- Message Passing
- POSIX/BSD Sockets
- TCP
- Python
- Matplotlib

## Project Information

**Institution:** Sreenidhi Institute of Science and Technology  
**Department:** Electronics and Communication Engineering

## Project Structure

The project consists of the following major modules:

- **Sensor Simulator** — generates simulated vehicle and environmental data.
- **Planner** — determines the required vehicle action.
- **Controller** — generates throttle and steering commands.
- **Actuator** — handles the final control stage and telemetry transmission.
- **Python Dashboard** — visualizes the actuator data received over TCP.