// actuator.c
// QNX Autonomous Vehicle Control Simulator
// Actuator process: receives final throttle/steering values from the
// Controller and streams them over TCP to a host-side dashboard.
//
// NOTE: Update SERVER_IP below to match your HOST machine's IP address
// on the network shared with the QNX VM (not the VM's own IP).
// Requires linking against the socket library: add -lsocket to the
// linker settings (Project Properties -> C/C++ Build -> Settings ->
// Linker -> Libraries), otherwise you will get "undefined reference
// to socket/connect/send" errors at link time.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct {
    float throttle;
    float steering;
} actuator_msg_t;

#define SERVER_PORT 5000
#define SERVER_IP "YOUR_HOST_IP"   // <-- set this to your host machine's IP

int main(void) {

    int chid;
    int sockfd = -1;

    struct sockaddr_in server_addr;

    actuator_msg_t act;

    chid = ChannelCreate(0);

    if (chid == -1) {
        perror("ChannelCreate failed");
        return EXIT_FAILURE;
    }

    printf("[Actuator] Channel created with ID: %d\n", chid);
    printf("[Actuator] PID: %d\n", getpid());

    /* TCP socket to host dashboard */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        perror("[Actuator] Socket creation failed");
    } else {

        memset(&server_addr, 0, sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);

        inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

        printf("[Actuator] Connecting to dashboard...\n");

        if (connect(sockfd,
                    (struct sockaddr *)&server_addr,
                    sizeof(server_addr)) == -1) {

            perror("[Actuator] TCP connection failed");
            close(sockfd);
            sockfd = -1;

        } else {

            printf("[Actuator] TCP connection established!\n");
        }
    }

    printf("[Actuator] Waiting for controller commands...\n");
    fflush(stdout);

    while (1) {

        int rcvid = MsgReceive(chid, &act, sizeof(act), NULL);

        if (rcvid == -1) {
            perror("MsgReceive failed");
            continue;
        }

        printf("[Actuator] Applying: throttle=%.2f%%, steering=%.2f deg\n",
               act.throttle,
               act.steering);

        fflush(stdout);

        /* Send actuator data to Python dashboard */
        if (sockfd != -1) {

            char buffer[100];

            int len = snprintf(buffer,
                               sizeof(buffer),
                               "%.2f,%.2f\n",
                               act.throttle,
                               act.steering);

            if (send(sockfd, buffer, len, 0) == -1) {

                perror("[Actuator] TCP send failed");

                close(sockfd);
                sockfd = -1;
            }
        }

        MsgReply(rcvid, EOK, NULL, 0);
    }

    return EXIT_SUCCESS;
}
