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

    printf("\n============================\n");
    //check file name to server
    outBuffer[0] = 0x03;
    outBuffer[1] = 0x00;
    outBuffer[2] = 0x00;
    outBuffer[3] = 0x01;
    outBuffer[4] = 0x04;
    send(socketFd, outBuffer, 5, 0);
    printf("Sent: ");
    for (int i = 0; i < 5; i++)
    {
        printf("%X", outBuffer[i]);
    }
    printf("\n");

    // Receive from server
    valread = recv(socketFd, inBuffer, BUFFER_SIZE, 0);
    printf("Recieved: ");
    for (int i = 0; i < 5; i++)
    {
        printf("%X", inBuffer[i]);
    }
    printf("\n");

    // Clean buffer
    memset(inBuffer, 0, sizeof(inBuffer));
    memset(outBuffer, 0, sizeof(outBuffer));
    printf("\n============================\n");
    printf("\nEnter message: ");

    close(socketFd);
    return 0;
}