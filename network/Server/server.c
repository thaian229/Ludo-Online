#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include "request.h"
#include "response.h"
#include "room.h"

#define BUFFER_SIZE 512
#define PORT 5500
#define BACKLOG 16

void *connection_handler(void *);

Room *createRoom(int socketFd, Room *curRoom, int newRoomId);

Room *quickJoin(int socketFd, Room *curRoom, int backupRoomId);

Room *joinARoom(int socketFd, Room *curRoom, int roomId);

void ready(int socketFd, Room *room);

void updateRoomStatus(int socketFd, Room *room, RoomStatus *roomStatus);

void initGame(int socketFd, Room *room, GameInitInfo *gameInitInfo);

void move(int socketFd, Room *room, Move *move);

void quitGame(int socketFd, Room *room);

void invalid(int socketFd);

Room *head;

int main(int argc, char const *argv[])
{
    // server, client and bytes count
    int listenFd, connectFd;
    int portNumber = 0;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen;
    int opt = 1;

    // Creating socket file descriptor
    if ((listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    portNumber = PORT;
    serverAddr.sin_port = htons(portNumber);

    // Bind socket with address and port
    if (bind(listenFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Bind failed!!!\n");
        exit(EXIT_FAILURE);
    }
    printf("Server binded at port %d\n", portNumber);

    // Start listen state
    if (listen(listenFd, BACKLOG) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Server is running... waiting for connection.\n");

    unsigned char buffer[20];
    buffer[0] = 0x01;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x01;
    buffer[4] = 0x04;
    printf("%d\n", (buffer[1] << 24 | buffer[2] << 16 | buffer[3] << 8 | buffer[4]));
    printf("%d\n", (buffer[4] << 24 | buffer[3] << 16 | buffer[2] << 8 | buffer[1]));
    // printf()

    Request *req = deserializeRequest(buffer);
    printf("completed\n");

    int no_threads = 0;
    pthread_t threads[BACKLOG];
    while (no_threads < BACKLOG)
    {
        clientLen = sizeof(clientAddr);
        connectFd = accept(listenFd, (struct sockaddr *)&clientAddr, (socklen_t *)&clientLen);

        puts("\nConnection accepted");
        printf("You got a connection from %s\n", inet_ntoa(clientAddr.sin_addr)); /* prints client's IP */

        if (pthread_create(&threads[no_threads], NULL, connection_handler, &connectFd) < 0)
        {
            perror("Could not create thread");
            return 1;
        }
        if (connectFd < 0)
        {
            printf("server acccept failed...\n");
            exit(0);
        }
        else
            printf("Server acccept the client...\n");
        puts("Handler assigned\n");
        no_threads++;
    }
    for (int k = 0; k < BACKLOG; k++)
    {
        pthread_join(threads[k], NULL);
    }

    // Close listening socket
    close(listenFd);
    return 0;
}

void *connection_handler(void *connectFd)
{
    int socket = *(int *)connectFd;
    int read_len;
    unsigned char inBuffer[BUFFER_SIZE];
    unsigned char outBuffer[BUFFER_SIZE];
    int valread;
    Room *room;

    while ((valread = read(socket, inBuffer, BUFFER_SIZE)) > 0)
    {
        // Terminate on empty
        if (inBuffer[0] == 0 || strcmp(inBuffer, "q") == 0 || strcmp(inBuffer, "Q") == 0)
        {
            printf("Connection closed...\n");
            break;
        }

        printf("\n============================\n");
        for (int i = 0; i < valread; i++)
        {
            printf("%X", inBuffer[i]);
        }
        printf("\n");

        Request *req = deserializeRequest(inBuffer);

        if (req->type != INVALID)
        {
            memcpy(outBuffer, inBuffer, valread);
            // Send
            send(socket, outBuffer, sizeof(outBuffer), 0);
            for (int i = 0; i < valread; i++)
            {
                printf("%X", inBuffer[i]);
            }
            printf("\n");
            printf("\n============================\n");
        }

        switch (req->type)
        {
        case CREATE_ROOM:
            room = createRoom(socket, room, pthread_self());
            break;

        case QUICK_JOIN:
            room = quickJoin(socket, room, pthread_self());
            //send back res
            break;

        case JOIN_A_ROOM:;
            printf("%d\n", req->roomId);
            room = joinARoom(socket, room, req->roomId);
            //send back res
            break;

        case READY:
            ready(socket, room);
            //send back res
            break;

        case MOVE:
            move(socket, room, req->move);
            //send back res
            break;

        case QUIT_GAME:
            quitGame(socket, room);
            //send back res
            break;

        default:
            invalid(socket);
            break;
        }

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
    close(socket);

    return 0;
}

Room *createRoom(int socketFd, Room *curRoom, int newRoomId)
{
    unsigned char *buffer;
    Response *res = (Response *)malloc(sizeof(Response));
    res->type = CREATE_ROOM_RESPONSE;

    if (curRoom == NULL)
    {
        Room *room = addRoom(head, newRoomId, socketFd);

        res->success = true;
        res->roomId = room->id;

        buffer = serializeResponse(res);

        send(socketFd, buffer, sizeof(buffer), 0);

        free(buffer);
        freeResponse(res);
        return room;
    }
    else
    {
        res->success = false;
        strcpy(res->err, "ALREADY IN ANOTHER ROOM");
        buffer = serializeResponse(res);
        send(socketFd, buffer, sizeof(buffer), 0);
        free(buffer);
        freeResponse(res);
        return curRoom;
    }
}

Room *quickJoin(int socketFd, Room *curRoom, int backupRoomId)
{
    unsigned char *buffer;
    Response *res = (Response *)malloc(sizeof(Response));
    res->type = QUICK_JOIN_RESPONSE;

    if (curRoom == NULL)
    {
        Room *room = quickJoinRoom(head, backupRoomId, socketFd);

        res->success = true;
        res->roomId = room->id;

        buffer = serializeResponse(res);

        send(socketFd, buffer, sizeof(buffer), 0);

        free(buffer);
        freeResponse(res);
        return room;
    }
    else
    {
        res->success = false;
        strcpy(res->err, "ALREADY IN ANOTHER ROOM");
        buffer = serializeResponse(res);
        send(socketFd, buffer, sizeof(buffer), 0);
        free(buffer);
        freeResponse(res);
        return curRoom;
    }
}

Room *joinARoom(int socketFd, Room *curRoom, int roomId)
{
    unsigned char *buffer;
    Response *res = (Response *)malloc(sizeof(Response));
    res->type = JOIN_A_ROOM_RESPONSE;

    if (curRoom == NULL)
    {
        Room *room = searchRoomById(head, roomId);
        if (room != NULL)
        {
            res->success = true;
            res->roomId = room->id;

            buffer = serializeResponse(res);

            send(socketFd, buffer, sizeof(buffer), 0);
        }
        else
        {
            res->success = false;
            strcpy(res->err, "ROOM NOT FOUND");
            buffer = serializeResponse(res);
            send(socketFd, buffer, sizeof(buffer), 0);
        }
        free(buffer);
        freeResponse(res);
        return room;
    }
    else
    {
        res->success = false;
        strcpy(res->err, "ALREADY IN ANOTHER ROOM");
        buffer = serializeResponse(res);
        send(socketFd, buffer, sizeof(buffer), 0);
        free(buffer);
        freeResponse(res);
        return curRoom;
    }
}

void ready(int socketFd, Room *room)
{
    unsigned char *buffer;
    Response *res = (Response *)malloc(sizeof(Response));
    res->type = READY_RESPONSE;

    if (room != NULL)
    {
        if (updateReadyStatus(room, socketFd))
        {
            res->success = true;
        }
        else
        {
            res->success = false;
            strcpy(res->err, "UPDATE FAILED");
        }
    }
    else
    {
        res->success = false;
        strcpy(res->err, "YOU ARE NOT IN A ROOM");
    }

    buffer = serializeResponse(res);
    send(socketFd, buffer, sizeof(buffer), 0);

    free(buffer);
    freeResponse(res);
}

void quitGame(int socketFd, Room *room)
{
}

void updateRoomStatus(int socketFd, Room *room, RoomStatus *roomStatus)
{
}

void initGame(int socketFd, Room *room, GameInitInfo *gameInitInfo)
{
}

void move(int socketFd, Room *room, Move *move)
{
    unsigned char *buffer;
    Response *res = (Response *)malloc(sizeof(Response));
    res->type = MOVE;

    if (move != NULL)
    {
        res->success = true;
        res->move = move;
        buffer = serializeResponse(res);

        for (int i = 0; i < 4; i++)
        {
            if (room->clientFd[i] != socketFd && room->clientFd[i] != 0)
            {
                send(room->clientFd[i], buffer, sizeof(buffer), 0);
            }
        }
    }
    else
    {
        res->success = false;
        strcpy(res->err, "MOVE BROKEN");
        buffer = serializeResponse(res);
        send(socketFd, buffer, sizeof(buffer), 0);
    }
    free(buffer);
    freeResponse(res);
}

void invalid(int socketFd)
{
    send(socketFd, "INVALID MESSAGE", sizeof("INVALID MESSAGE"), 0);
}