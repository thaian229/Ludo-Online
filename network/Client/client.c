// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define BUFFER_SIZE 512
#define PORT 5500
#define IP "127.0.0.1"

void *send_handler(void *);

void *recv_handler(void *);

int main(int argc, char const *argv[])
{
    int socketFd, valread;
    struct sockaddr_in serverAddr;
    char inBuffer[BUFFER_SIZE] = {0};
    unsigned char outBuffer[BUFFER_SIZE] = {0};
    int portNumber;
    long byteSent = 0;

    // Creating socket file descriptor
    if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Setup socket
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    portNumber = PORT;
    serverAddr.sin_port = htons(portNumber);

    //Connection of the client to the socket
    if (connect(socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("[-]Problem in connecting to the server\n");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[2];
    while (1)
    {
        if (pthread_create(&threads[0], NULL, send_handler, &socketFd) < 0)
        {
            perror("Could not create thread");
            return 1;
        }
        if (pthread_create(&threads[0], NULL, recv_handler, &socketFd) < 0)
        {
            perror("Could not create thread");
            return 1;
        }
        printf("Connected to the server...\n");
    }

    close(socketFd);
    return 0;
}

void *send_handler(void *socketFd)
{
    int socket = *(int *)socketFd;
    unsigned char outBuffer[BUFFER_SIZE];
    int valread;
    Room *room;
}

void *recv_handler(void *socketFd)
{
    int socket = *(int *)socketFd;
    unsigned char inBuffer[BUFFER_SIZE];
    unsigned char outBuffer[BUFFER_SIZE];
    int valread;
    Room *room;
}