"""
dashboard.py
QNX Autonomous Vehicle Control Simulator - Host-side live dashboard.

Listens on a TCP socket for "throttle,steering" lines sent by the
QNX Actuator process and plots them live using matplotlib.

Run this BEFORE starting the QNX processes (Actuator -> Controller ->
Planner -> Sensor, in that order), since the Actuator connects out to
this listener on startup.

    pip install matplotlib
    python dashboard.py
"""

import socket
import threading
from collections import deque

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

HOST = "0.0.0.0"
PORT = 5000

# Rolling buffers for the most recent readings
throttle_data = deque(maxlen=50)
steering_data = deque(maxlen=50)


def socket_listener():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((HOST, PORT))
    server.listen(1)
    print("Waiting for QNX Actuator...")

    conn, addr = server.accept()
    print("Connected from:", addr)

    buffer = ""
    while True:
        data = conn.recv(1024)
        if not data:
            break
        buffer += data.decode()

        while "\n" in buffer:
            line, buffer = buffer.split("\n", 1)
            line = line.strip()
            if not line:
                continue
            try:
                throttle, steering = map(float, line.split(","))
                throttle_data.append(throttle)
                steering_data.append(steering)
            except ValueError:
                pass

    conn.close()
    server.close()


# Run socket listener in a background thread so plotting isn't blocked
listener_thread = threading.Thread(target=socket_listener, daemon=True)
listener_thread.start()

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 6))


def update(frame):
    ax1.clear()
    ax2.clear()

    ax1.plot(list(throttle_data), color="green")
    ax1.set_title("Throttle (%)")
    ax1.set_ylim(0, 100)

    ax2.plot(list(steering_data), color="blue")
    ax2.set_title("Steering Angle (deg)")
    ax2.set_ylim(-30, 30)

    plt.tight_layout()


ani = FuncAnimation(fig, update, interval=500)
plt.show()
