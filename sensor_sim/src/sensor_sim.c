// sensor_sim.c
// QNX Autonomous Vehicle Control Simulator
// Sensor Simulator process: generates a scripted obstacle/lane-drift
// scenario and sends it to the Planner process via QNX native IPC.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <time.h>

typedef struct {
    float obstacle_distance;
    float lane_offset;
    float current_speed;
} sensor_msg_t;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <planner_pid>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t planner_pid = atoi(argv[1]);
    int coid = ConnectAttach(0, planner_pid, 1, 0, 0);

    if (coid == -1) {
        perror("ConnectAttach failed");
        return EXIT_FAILURE;
    }

    printf("[Sensor] Connected to planner (PID %d)\n", planner_pid);
    fflush(stdout);

    // Scripted scenario: obstacle approach -> brake -> clear -> lane drift -> correct
    float distance_sequence[] = {
        10.0, 8.0, 6.0, 4.0, 2.5, 1.5,   // obstacle approaching -> BRAKE
        10.0, 10.0, 10.0,                 // road clear again
        10.0, 10.0, 10.0                  // lane drift scenario
    };

    float offset_sequence[] = {
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.3, 0.7,                    // drifting -> STEER_CORRECT
        0.6, 0.2, 0.0                     // correcting back
    };

    float speed_sequence[] = {
        30, 28, 25, 20, 15, 10,
        25, 25, 25,
        25, 25, 25
    };

    int steps = sizeof(distance_sequence) / sizeof(float);
    sensor_msg_t msg;

    while (1) {  // loop the scenario continuously for repeated demo runs
        for (int i = 0; i < steps; i++) {
            msg.obstacle_distance = distance_sequence[i];
            msg.lane_offset = offset_sequence[i];
            msg.current_speed = speed_sequence[i];

            if (MsgSend(coid, &msg, sizeof(msg), NULL, 0) == -1) {
                perror("MsgSend failed");
            } else {
                printf("[Sensor] Step %d: dist=%.2f, offset=%.2f, speed=%.2f\n",
                       i, msg.obstacle_distance, msg.lane_offset, msg.current_speed);
                fflush(stdout);
            }

            sleep(1);
        }

        printf("[Sensor] Scenario complete. Restarting...\n");
        fflush(stdout);
        sleep(2); // brief pause before looping again
    }

    return EXIT_SUCCESS;
}
