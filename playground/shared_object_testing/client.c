// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096

int socketFd, valread;
struct sockaddr_in serverAddr;
char inBuffer[BUFFER_SIZE] = {0};
char outBuffer[BUFFER_SIZE] = {0};
int portNumber;

char *sendMsg(char msg[BUFFER_SIZE])
{
	strcpy(outBuffer, msg);

	// Terminated on empty input
	if (outBuffer[0] == '\n')
	{
		send(socketFd, outBuffer, sizeof(outBuffer), 0);
		printf("Terminating...\n");
	}

	// Remove newline first
	outBuffer[strcspn(outBuffer, "\n")] = 0;

	// Shutdown both client and server
	if (strcmp(outBuffer, "SHUTDOWN") == 0)
	{
		send(socketFd, outBuffer, sizeof(outBuffer), 0);
		printf("Terminating client...\n");
	}

	send(socketFd, outBuffer, sizeof(outBuffer), 0);
	printf("Sent: %s\n", outBuffer);

	// Receive from server
	valread = recv(socketFd, inBuffer, BUFFER_SIZE, 0);
	printf("Recieved: %s\n", inBuffer);

	char *greeting = malloc(sizeof(char) * (strlen(inBuffer) + 1));
	strcpy(greeting, inBuffer);
	printf("Return to Python: %s\n", greeting);

	// Clean buffer
	memset(inBuffer, 0, sizeof(inBuffer));
	memset(outBuffer, 0, sizeof(outBuffer));

	return greeting;
}

int connecting()
{
	// Creating socket file descriptor
	if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Setup socket
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	portNumber = 5500;
	serverAddr.sin_port = htons(portNumber);

	//Connection of the client to the socket
	if (connect(socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("Problem in connecting to the server\n");
		exit(EXIT_FAILURE);
	}

	printf("Socket in C: %d\n", socketFd);

	return socketFd;
}