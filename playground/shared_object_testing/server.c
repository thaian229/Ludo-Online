// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 4096
#define LISTENQ 8

int splitLetterAndDigit(char *in, char *digit, char *letter);

int main(int argc, char const *argv[])
{
    // check arguments
    if (argc != 2)
    {
        perror("Require port number for environment variable.\n");
        exit(EXIT_FAILURE);
    }

    // server, client and bytes count
    int listenFd, connectFd, valread;
    int portNumber = 0;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen;
    int opt = 1;
    char inBuffer[BUFFER_SIZE] = {0};
    char outBuffer[BUFFER_SIZE] = {0};

    // Creating socket file descriptor
    if ((listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port even if it being used, <<< Use With Caution !!! >>>
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (!atoi(argv[1]))
    {
        perror("Invalid port number!!!\n");
        exit(EXIT_FAILURE);
    }
    portNumber = atoi(argv[1]);
    serverAddr.sin_port = htons(portNumber);

    // Bind socket with address and port
    if (bind(listenFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Bind failed!!!\n");
        exit(EXIT_FAILURE);
    }
    printf("Server binded at port %d\n", portNumber);

    // Start listen state
    if (listen(listenFd, LISTENQ) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Server is running... waiting for connection.\n");

    for (;;)
    {
        clientLen = sizeof(clientAddr);

        // accept connection
        if ((connectFd = accept(listenFd, (struct sockaddr *)&clientAddr, (socklen_t *)&clientLen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Connected.\n\n");

        while ((valread = recv(connectFd, inBuffer, BUFFER_SIZE, 0)) > 0)
        {
            // Terminate on empty
            if (inBuffer[0] == '\n' || inBuffer[0] == 0)
            {
                printf("Connection closed\nWaiting...\n");
                break;
            }

            // Completely shutdown server
            if (strcmp(inBuffer, "SHUTDOWN") == 0)
            {
                close(connectFd);
                close(listenFd);
                printf("SERVER SHUTTING DOWN...\n");
                sleep(1);
                printf("SERVER SHUTDOWNED.\n");
                exit(EXIT_SUCCESS);
            }

            printf("Received: %s\n", inBuffer);

            // Main service
            char digitOnly[BUFFER_SIZE] = {0};
            char letterOnly[BUFFER_SIZE] = {0};

            if (splitLetterAndDigit(inBuffer, digitOnly, letterOnly) < 0)
            {
                strcpy(outBuffer, "Error");
            }
            else
            {
                if (*digitOnly != 0 && *letterOnly != 0)
                {
                    strcpy(outBuffer, "\n");
                    strcat(outBuffer, digitOnly);
                    strcat(outBuffer, "\n");
                    strcat(outBuffer, letterOnly);
                }
                else
                {
                    strcpy(outBuffer, digitOnly);
                    strcat(outBuffer, letterOnly);
                }
            }

            // Send
            send(connectFd, outBuffer, sizeof(outBuffer), 0);
            printf("Sent: %s\n\n", outBuffer);

            // Clean buffer
            memset(inBuffer, 0, sizeof(inBuffer));
            memset(outBuffer, 0, sizeof(outBuffer));
        }

        if (valread < 0)
        {
            perror("Read error");
            exit(EXIT_FAILURE);
        }

        // Close connected socket
        close(connectFd);
    }

    // Close listening socket
    close(listenFd);
    return 0;
}

int splitLetterAndDigit(char *in, char *digit, char *letter)
{
    char *x;
    char *d = digit;
    char *l = letter;
    for (x = in; *x != '\n' && *x; x++)
    {
        if (isdigit(*x))
        {
            *d = *x;
            d++;
        }
        else if (isalpha(*x))
        {
            *l = *x;
            l++;
        }
        else
        {
            return -1;
        }
    }
    *d = 0;
    *l = 0;
    return 0;
}