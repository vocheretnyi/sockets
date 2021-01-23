#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/un.h>
#include <stdbool.h>

#define PACKET_SIZE 1024
const int SOCKET_PORT = 8080;
const char *SOCKET_FILE = "./socket";

enum Socket {
    UNIX,
    INET
};

struct sockaddr_in serv_in_addr;
struct sockaddr_un serv_un_addr;

int open_unix(bool is_blocking, struct sockaddr_un *serv_addr) {

    const int socket_file_descriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1) {
        printf("Error during creating socket\n");
        exit(-1);
    }

    if (!is_blocking) {
        fcntl(socket_file_descriptor, F_SETFL, O_NONBLOCK);
    }

    serv_addr->sun_family = AF_UNIX;
    strcpy(serv_addr->sun_path, SOCKET_FILE);
    unlink(SOCKET_FILE);

    if (bind(socket_file_descriptor, (struct sockaddr *) serv_addr,
             strlen(serv_addr->sun_path) + sizeof(serv_addr->sun_family)) < 0) {
        printf("Binding failed\n");
        exit(-1);
    } else {
        printf("Binding successful!\n");
    }

    if (listen(socket_file_descriptor, 10) < 0) {
        printf("Listen failed\n");
        exit(-1);
    } else {
        printf("Listening ...\n");
    }

    return socket_file_descriptor;
}

int open_inet(bool is_blocking, struct sockaddr_in *serv_addr) {

    const int socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1) {
        printf("Error during creating socket\n");
        exit(-1);
    }

    if (!is_blocking) {
        fcntl(socket_file_descriptor, F_SETFL, O_NONBLOCK);
    }

    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr->sin_port = htons(SOCKET_PORT);

    if (bind(socket_file_descriptor, (struct sockaddr *) serv_addr, sizeof(*serv_addr)) < 0) {
        printf("Binding failed\n");
        exit(-1);
    } else {
        printf("Binding successful!\n");
    }

    if (listen(socket_file_descriptor, 10) < 0) {
        printf("Listen failed\n");
        exit(-1);
    } else {
        printf("Listening ...\n");
    }

    return socket_file_descriptor;
}

int open_socket(enum Socket socket_type, bool is_blocking, struct sockaddr *serv_addr) {

    serv_addr = NULL;

    if (socket_type == UNIX) {
        const int socket_file_descriptor = open_unix(is_blocking, &serv_un_addr);
        serv_addr = (struct sockaddr *) &serv_un_addr;
        return socket_file_descriptor;
    } else if (socket_type == INET) {
        const int socket_file_descriptor = open_inet(is_blocking, &serv_in_addr);
        serv_addr = (struct sockaddr *) &serv_in_addr;
        return socket_file_descriptor;
    }

    return -1;
}

int main(int argc, char *argv[]) {

    enum Socket socket_type = UNIX;
    bool is_blocking = false;

    if (argc == 3) {
        if (strcmp(argv[1], "UNIX") == 0) {
            socket_type = UNIX;
        } else if (strcmp(argv[1], "INET") == 0) {
            socket_type = INET;
        } else {
            printf("Passed wrong or unsupported socket type.\n");
            exit(0);
        }

        if (strcmp(argv[2], "0") == 0) {
            is_blocking = false;
        } else if (strcmp(argv[2], "1") == 0) {
            is_blocking = true;
        } else {
            printf("Passed wrong blocking type.\n");
            exit(0);
        }
    } else {
        printf("Passed wrong number of parameters.\n");
        exit(0);
    }

    char data_buffer[PACKET_SIZE];

    struct sockaddr *serv_addr = NULL;
    const int addrlen = sizeof(*serv_addr);

    printf("Opening socket ...\n");
    int socket_file_descriptor = open_socket(socket_type, is_blocking, serv_addr);

    while (1) {
        const int new_socket = accept(socket_file_descriptor, serv_addr, (socklen_t *) &addrlen);
        if (new_socket == -1) {
            if (is_blocking == 1) {
                printf("Error during accept new socket\n");
                exit(-1);
            } else {
                printf("Waiting for the connection ...\n");
                sleep(1);
            }
            continue;
        }
        printf("New connection!\n");
        uint64_t total = 0;
        while (1) {
            const int received_data_len = read(new_socket, data_buffer, sizeof(data_buffer));
            total += received_data_len;
            if (received_data_len == 0) {
                printf("Received empty packet\n");
                break;
            } else {
                printf("Received data_buffer with packet size = %d\n", received_data_len);
            }
        }
        printf("Total received bytes from socket = %lu\n", total);
        close(new_socket);

    }

    return 0;
}