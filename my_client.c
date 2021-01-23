#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>

const char *SOCKET_FILE = "socket";
const int SOCKET_PORT = 8080;
const int PACKET_SIZE = 1 << 12;

enum Socket {
    UNIX,
    INET
};

uint64_t startTime;
uint64_t finishTime;
#define TimerWrapper(name, expr) \
    startTime = getCurTime(); \
    expr; \
    finishTime = getCurTime(); \
    printf("%s took: %.06f\n", name, (finishTime - startTime) / 1000000.0f); \


static uint64_t getCurTime() {
    struct timespec tms;
    if (clock_gettime(CLOCK_REALTIME, &tms)) {
        return -1;
    }
    return tms.tv_sec * 1000000 + tms.tv_nsec / 1000;
}

int open_inet() {

    const int socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1) {
        printf("Error during creating socket\n");
        exit(-1);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SOCKET_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported \n");
        exit(1);
    }

    if (connect(socket_file_descriptor, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        printf("Error during connecting socket\n");
        exit(-1);
    }

    return socket_file_descriptor;
}

int open_unix() {

    const int socket_file_descriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1) {
        printf("Error during creating socket\n");
        exit(-1);
    }

    struct sockaddr_un serv_addr;
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SOCKET_FILE);

    if (connect(socket_file_descriptor, (struct sockaddr *) &serv_addr,
                strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family)) == -1) {
        printf("Error during connecting socket\n");
        exit(-1);
    }

    return socket_file_descriptor;
}

int open_socket(enum Socket socket_type) {

    if (socket_type == UNIX) {
        return open_unix();
    } else if (socket_type == INET) {
        return open_inet();
    }

    return -1;
}

bool send_data(int socket_file_descriptor, const char *data, const int data_len) {

    int sent_data = write(socket_file_descriptor, data, data_len);
    if (sent_data != data_len) {
        return false;
    }
    return true;
}

char *generate_data(const int len) {

    char *data = malloc(len);
    for (int i = 0; i < len; ++i) {
        data[i] = rand() % 26 + 'a';
    }
    return data;
}


int main(int argc, char *argv[]) {

    enum Socket socket_type = UNIX;

    if (argc == 2) {
        if (strcmp(argv[1], "UNIX") == 0) {
            printf("UNIX\n");
            socket_type = UNIX;
        } else if (strcmp(argv[1], "INET") == 0) {
            socket_type = INET;
        } else {
            printf("Passed wrong or unsupported socket type.\n");
            exit(0);
        }
    } else {
        printf("Passed wrong number of parameters.\n");
        exit(0);
    }

    printf("Generating data to send ...\n");
    char *data = generate_data(PACKET_SIZE);

    int socket_file_descriptor;

    TimerWrapper("open socket", {
        printf("Opening socket ...\n");
        socket_file_descriptor = open_socket(socket_type);
    });

    TimerWrapper("send data", {
        printf("Sending the data from client ...\n");
        if (send_data(socket_file_descriptor, data, PACKET_SIZE)) {
            printf("Successfully sent %d of data\n", PACKET_SIZE);
        } else {
            printf("Error during sending the data\n");
        }
    });

    TimerWrapper("close socket", {
        printf("Closing socket.\n");
        close(socket_file_descriptor);
    });

    return 0;
}