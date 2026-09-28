// planner.c
// QNX Autonomous Vehicle Control Simulator
// Planner process: receives sensor data, applies threshold-based
// decision logic, and forwards a command to the Controller process.

#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
    float obstacle_distance;
    float lane_offset;
    float current_speed;
} sensor_msg_t;

typedef enum { BRAKE, STEER_CORRECT, MAINTAIN } command_t;

typedef struct {
    command_t command;
    float target_speed;
    float steer_angle;
} command_msg_t;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <controller_pid>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t controller_pid = atoi(argv[1]);
    int controller_coid = ConnectAttach(0, controller_pid, 1, 0, 0);
    if (controller_coid == -1) {
        perror("ConnectAttach to controller failed");
        return EXIT_FAILURE;
    }
    printf("[Planner] Connected to controller (PID %d)\n", controller_pid);
    fflush(stdout);

    int chid;
    sensor_msg_t msg;

    chid = ChannelCreate(0);
    if (chid == -1) {
        perror("ChannelCreate failed");
        return EXIT_FAILURE;
    }

    printf("[Planner] Channel created with ID: %d\n", chid);
    printf("[Planner] PID: %d\n", getpid());
    printf("[Planner] Waiting for sensor data...\n");
    fflush(stdout);

    while (1) {
        int rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) {
            perror("MsgReceive failed");
            continue;
        }

        printf("[Planner] Received: dist=%.2f, offset=%.2f, speed=%.2f\n",
               msg.obstacle_distance, msg.lane_offset, msg.current_speed);

        command_msg_t cmd;

        if (msg.obstacle_distance < 2.0) {
            cmd.command = BRAKE;
            cmd.target_speed = 0;
            cmd.steer_angle = 0;
        } else if (msg.lane_offset > 0.5 || msg.lane_offset < -0.5) {
            cmd.command = STEER_CORRECT;
            cmd.target_speed = msg.current_speed;
            cmd.steer_angle = -msg.lane_offset;
        } else {
            cmd.command = MAINTAIN;
            cmd.target_speed = msg.current_speed;
            cmd.steer_angle = 0;
        }

        printf("[Planner] Decision: cmd=%d, target_speed=%.2f, steer=%.2f\n",
               cmd.command, cmd.target_speed, cmd.steer_angle);
        fflush(stdout);

        // Send decision to Controller
        if (MsgSend(controller_coid, &cmd, sizeof(cmd), NULL, 0) == -1) {
            perror("MsgSend to controller failed");
        }

        // Reply to sensor (mandatory in QNX message passing)
        MsgReply(rcvid, EOK, NULL, 0);
    }

    return EXIT_SUCCESS;
}
