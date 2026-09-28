# QNX Autonomous Vehicle Control Simulator

A software-in-the-loop autonomous vehicle control simulation built using the **QNX Neutrino Real-Time Operating System**. The project models a simplified vehicle control pipeline in which sensor data is processed through independent QNX processes for planning, control, and actuation, with a Python dashboard for real-time visualization.

## Overview

Autonomous vehicle systems require multiple software components to exchange sensor information and control decisions with predictable timing and reliable inter-process communication.

This project demonstrates a simplified version of that architecture using **QNX native message-passing IPC**.

The system consists of four independent QNX processes:

1. **Sensor Simulator** — generates simulated vehicle and environment data.
2. **Planner** — evaluates sensor data and selects a driving action.
3. **Controller** — converts the selected action into throttle and steering commands.
4. **Actuator** — receives the control commands and forwards telemetry to a host dashboard.

A Python-based dashboard receives the actuator data through TCP and provides a visual representation of the vehicle control state.

## System Architecture

```text
                  ┌─────────────────────┐
                  │   Sensor Simulator  │
                  │                     │
                  │ • Obstacle distance │
                  │ • Lane offset       │
                  │ • Vehicle speed     │
                  └──────────┬──────────┘
                             │
                        QNX IPC
                             │
                             ▼
                  ┌─────────────────────┐
                  │       Planner       │
                  │                     │
                  │ • BRAKE             │
                  │ • STEER_CORRECT     │
                  │ • MAINTAIN          │
                  └──────────┬──────────┘
                             │
                        QNX IPC
                             │
                             ▼
                  ┌─────────────────────┐
                  │     Controller      │
                  │                     │
                  │ • Throttle          │
                  │ • Steering angle    │
                  └──────────┬──────────┘
                             │
                        QNX IPC
                             │
                             ▼
                  ┌─────────────────────┐
                  │      Actuator       │
                  │                     │
                  │ • Applies commands  │
                  │ • Sends telemetry   │
                  └──────────┬──────────┘
                             │
                             │ TCP
                             ▼
                  ┌─────────────────────┐
                  │  Python Dashboard   │
                  │                     │
                  │ • Throttle display  │
                  │ • Steering display  │
                  └─────────────────────┘ 
                                        
 ```
                
## Key Technologies

- **QNX Neutrino RTOS**
- **C**
- **QNX Native IPC**
- **Message Passing**
- **POSIX/BSD Sockets**
- **TCP Communication**
- **Python**
- **Matplotlib**
- **Software-in-the-loop simulation**

## System Components

### 1. Sensor Simulator

The sensor simulator generates representative vehicle and environment parameters:

- Obstacle distance
- Lane offset
- Current vehicle speed

The generated sensor message is transmitted to the planner using QNX message passing.

Source:

```text
sensor_sim/src/sensor_sim.c
```
### 2. Planner

The planner evaluates the incoming sensor information and determines the required driving action.

The current decision states are:

- `BRAKE`
- `STEER_CORRECT`
- `MAINTAIN`

The selected action is sent to the controller through QNX IPC.

Source:

```text
planner/src/planner.c
```

### 3. Controller

The controller converts the planner's decision into low-level vehicle control values.

The generated control parameters include:

- Throttle
- Steering angle

The controller uses proportional control logic for steering correction and sends the resulting commands to the actuator.

Source:

```text
controller/src/controller.c
```

### 4. Actuator

The actuator represents the final stage of the simulated control pipeline.

It receives the controller output through QNX IPC and transmits the resulting vehicle state to the host machine using TCP sockets.

Source:

```text
actuator/src/actuator.c
```

> **Configuration:** The actuator contains a configurable `SERVER_IP` value. Set this to the host machine IP reachable from the QNX environment when running the simulation.

### 5. Python Dashboard

The dashboard receives telemetry from the actuator and displays the current:

- Throttle
- Steering angle

Source:

```text
dashboard/dashboard.py
```

## QNX IPC Design

The project uses QNX's native synchronous message-passing mechanism to connect the individual processes.

The communication flow is:

```text
Sensor Simulator
       │
       │ MsgSend()
       ▼
     Planner
       │
       │ MsgSend()
       ▼
   Controller
       │
       │ MsgSend()
       ▼
    Actuator
```

The processes use QNX primitives including:

- `ChannelCreate()`
- `ConnectAttach()`
- `MsgSend()`
- `MsgReceive()`
- `MsgReply()`

This allows the project to demonstrate process-to-process communication using QNX's microkernel IPC model.

## Project Structure

```text
qnx-av-simulator/
│
├── README.md
├── .gitignore
│
├── sensor_sim/
│   └── src/
│       └── sensor_sim.c
│
├── planner/
│   └── src/
│       └── planner.c
│
├── controller/
│   └── src/
│       └── controller.c
│
├── actuator/
│   └── src/
│       └── actuator.c
│
├── dashboard/
│   └── dashboard.py
│
└── docs/
    └── abstract.md
```

## Running the Simulation

The QNX components are intended to run as separate processes in a QNX Neutrino environment.

The general execution sequence is:

```text
1. Start the Planner
2. Start the Controller
3. Start the Actuator
4. Start the Sensor Simulator
5. Start the Python Dashboard on the host machine
```

The exact build and execution commands depend on the QNX development environment and project configuration.

### Python Dashboard

Install the dashboard dependency:

```bash
pip install matplotlib
```

Then run:

```bash
python dashboard.py
```

### Network Configuration

The actuator communicates with the dashboard using TCP.

Before running the complete system, update the `SERVER_IP` definition in:

```text
actuator/src/actuator.c
```

with the IP address of the host machine that is reachable from the QNX environment.

Do not commit private or machine-specific network configuration to a public repository.

## Simulation Logic

A simplified control flow is used to demonstrate the interaction between sensing, planning and control.

For example:

```text
Obstacle too close
        ↓
      BRAKE
        ↓
Reduced throttle
```

and:

```text
Lane offset detected
        ↓
 STEER_CORRECT
        ↓
Steering adjustment
```

Otherwise:

```text
Normal conditions
        ↓
     MAINTAIN
        ↓
Maintain vehicle state
```

## Learning Objectives

This project provides practical exposure to:

- QNX Neutrino process architecture
- Real-time operating system concepts
- Inter-process communication
- Synchronous message passing
- Embedded control logic
- TCP socket communication
- Software-in-the-loop simulation
- Host-side telemetry visualization

## Future Improvements

Possible extensions include:

- Integration with real vehicle sensor interfaces
- More detailed vehicle dynamics
- Additional safety states
- Real-time performance measurements
- Hardware-in-the-loop testing
- CAN communication
- More advanced control algorithms
- Expanded telemetry and logging

## Project Information

**Institution:** Sreenidhi Institute of Science and Technology  
**Department:** Electronics and Communication Engineering

---

This repository is intended as a learning and development project demonstrating QNX-based process communication and a simplified autonomous vehicle control pipeline.