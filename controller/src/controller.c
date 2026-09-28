// controller.c
// QNX Autonomous Vehicle Control Simulator
// Controller process: receives high-level commands from the Planner,
// applies a proportional control law, and forwards actuator values
// to the Actuator process.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <errno.h>

typedef enum { BRAKE, STEER_CORRECT, MAINTAIN } command_t;

typedef struct {
    command_t command;
    float target_speed;
    float steer_angle;
} command_msg_t;

typedef struct {
    float throttle;
    float steering;
} actuator_msg_t;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <actuator_pid>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t actuator_pid = atoi(argv[1]);
    int actuator_coid = ConnectAttach(0, actuator_pid, 1, 0, 0);
    if (actuator_coid == -1) {
        perror("ConnectAttach to actuator failed");
        return EXIT_FAILURE;
    }
    printf("[Controller] Connected to actuator (PID %d)\n", actuator_pid);
    fflush(stdout);

    int chid;
    command_msg_t cmd_msg;

    chid = ChannelCreate(0);
    if (chid == -1) {
        perror("ChannelCreate failed");
        return EXIT_FAILURE;
    }

    printf("[Controller] Channel created with ID: %d\n", chid);
    printf("[Controller] PID: %d\n", getpid());
    printf("[Controller] Waiting for planner commands...\n");
    fflush(stdout);

    float Kp_speed = 1.0;
    float Kp_steer = 1.0;

    while (1) {
        int rcvid = MsgReceive(chid, &cmd_msg, sizeof(cmd_msg), NULL);
        if (rcvid == -1) {
            perror("MsgReceive failed");
            continue;
        }

        actuator_msg_t act;

        switch (cmd_msg.command) {
            case BRAKE:
                act.throttle = 0.0;
                act.steering = 0.0;
                break;
            case STEER_CORRECT:
                act.throttle = Kp_speed * cmd_msg.target_speed * 0.5;
                act.steering = Kp_steer * cmd_msg.steer_angle * 15.0;
                break;
            case MAINTAIN:
            default:
                act.throttle = Kp_speed * cmd_msg.target_speed * 0.5;
                act.steering = 0.0;
                break;
        }

        printf("[Controller] cmd=%d -> throttle=%.2f%%, steering=%.2f deg\n",
               cmd_msg.command, act.throttle, act.steering);
        fflush(stdout);

        // Send to Actuator
        if (MsgSend(actuator_coid, &act, sizeof(act), NULL, 0) == -1) {
            perror("MsgSend to actuator failed");
        }

        // Reply to Planner
        MsgReply(rcvid, EOK, NULL, 0);
    }

    return EXIT_SUCCESS;
}
