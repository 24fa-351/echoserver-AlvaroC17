#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h> 

void *handleConnection(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[1024];
    int bytes_read;

    while((bytes_read = read(sock, buffer, sizeof(buffer)-1)) > 0) {
        buffer[bytes_read] = '\0'; 
        write(sock, buffer, bytes_read);
    }

    if(bytes_read == 0) {
        puts("Client disconnected");
    }
    else {
        perror("recv failed");
    }

    close(sock);
    free(socket_desc);

    return 0;
}

int main(int argc, char *argv[]) {
    int socket_fd, client_fd, *new_sock;
    struct sockaddr_in server, client;
    int c = sizeof(struct sockaddr_in);
    int port = 46645; 

    int opt;
    while((opt = getopt(argc, argv, "p:")) != -1) {
        switch(opt) {
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -p port\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed");
        return 1;
    }

    listen(socket_fd, 3);

    puts("Waiting for incoming connections...");
    while((client_fd = accept(socket_fd, (struct sockaddr *)&client, (socklen_t*)&c))) {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_fd;

        if(pthread_create(&sniffer_thread, NULL, handleConnection, (void*) new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }
        puts("Handler assigned");
    }

    if (client_fd < 0) {
        perror("accept failed");
        return 1;
    }

    return 0;
}
