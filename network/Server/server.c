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
#define MAXROOM 256

void *connection_handler(void *);

Room *createRoom(int socketFd, Room *curRoom);

Room *quickJoin(int socketFd, Room *curRoom);

Room *joinARoom(int socketFd, Room *curRoom, int roomId);

void ready(int socketFd, Room *room);

void updateRoomStatus(int socketFd, Room *room);

void initGame(int socketFd, Room *room);

void move(int socketFd, Room *room, Move *move);

void quitGame(int socketFd, Room *room);

void invalid(int socketFd);

int findEmptyRoomId();

Room *head;

int roomIds[MAXROOM] = {0};

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
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
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
    // unsigned char *outBuffer;
    int valread;
    Room *room;

    while ((valread = read(socket, inBuffer, sizeof(inBuffer))) > 0)
    {
        // Terminate on empty
        if (inBuffer[0] == 0 || strcmp(inBuffer, "q") == 0 || strcmp(inBuffer, "Q") == 0)
        {
            printf("Connection closed...\n");
            break;
        }

        printf("\n============================\n\n");
        for (int i = 0; i < valread; i++)
        {
            printf("%X", inBuffer[i]);
        }

        printf("\n");

        Request *req = deserializeRequest(inBuffer);

        switch (req->type)
        {
        case CREATE_ROOM:

            room = createRoom(socket, room);
            // updateRoomStatus(socket, room);
            break;

        case QUICK_JOIN:
            room = quickJoin(socket, room);
            // updateRoomStatus(socket, room);
            break;

        case JOIN_A_ROOM:;
            printf("%d\n", req->roomId);
            room = joinARoom(socket, room, req->roomId);
            // updateRoomStatus(socket, room);
            break;

        case READY:
            ready(socket, room);
            updateRoomStatus(socket, room);
            if (calculateNumberOfReadiedClient(room) >= 4)
            {
                initGame(socket, room);
                room->isPlaying = true;
            }
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
        // memset(outBuffer, 0, sizeof(outBuffer));
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

Room *createRoom(int socketFd, Room *curRoom)
{
    unsigned char *buffer;
    Response *res = (Response *)malloc(sizeof(Response));
    res->type = CREATE_ROOM_RESPONSE;

    if (curRoom == NULL)
    {
        int roomId = findEmptyRoomId();
        Room *room = addRoom(head, roomId, socketFd);
        roomIds[roomId] = 1;

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

Room *quickJoin(int socketFd, Room *curRoom)
{
    unsigned char *buffer;
    Response *res = (Response *)malloc(sizeof(Response));
    res->type = QUICK_JOIN_RESPONSE;

    if (curRoom == NULL)
    {
        int roomId = findEmptyRoomId();
        Room *room = quickJoinRoom(head, roomId, socketFd);
        roomIds[roomId] = 1;

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

void updateRoomStatus(int socketFd, Room *room)
{
    if (room != NULL)
    {
        unsigned char *buffer;
        Response *res = (Response *)malloc(sizeof(Response));
        res->type = ROOM_STATUS_UPDATE;
        res->success = true;

        RoomStatus *status = (RoomStatus *)malloc(sizeof(RoomStatus));
        status->players = calculateNumberOfClientInRoom(room);
        status->ready = calculateNumberOfReadiedClient(room);

        res->roomStatus = status;

        buffer = serializeResponse(res);

        send(socketFd, buffer, sizeof(buffer), 0);

        free(buffer);
        // freeResponse(res);
    }
}

void initGame(int socketFd, Room *room)
{
    unsigned char *buffer;
    Response *res = (Response *)malloc(sizeof(Response));
    res->type = GAME_INIT;
    res->success = true;

    GameInitInfo *gi = (GameInitInfo *)malloc(sizeof(GameInitInfo));
    for (int i = 0; i < calculateNumberOfClientInRoom(room); i++)
    {
        if (socketFd == room->clientFd[i])
        {
            gi->yourColor = i;
        }
    }

    res->gameInitInfo = gi;

    buffer = serializeResponse(res);

    send(socketFd, buffer, sizeof(buffer), 0);

    free(buffer);
    freeResponse(res);
}

void move(int socketFd, Room *room, Move *move)
{
    unsigned char *buffer;
    Response *res = (Response *)malloc(sizeof(Response));
    res->type = MOVE;

    if (move != NULL && room->isPlaying == true)
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
        strcpy(res->err, "INVALID MOVE");
        buffer = serializeResponse(res);
        send(socketFd, buffer, sizeof(buffer), 0);
    }
    free(buffer);
    freeResponse(res);
}

void invalid(int socketFd)
{
    unsigned char *buffer;
    Response *res = (Response *)malloc(sizeof(Response));
    res->type = INVALID;
    res->success = false;
    strcpy(res->err, "INVALID MESSSAGE TYPE");
    free(buffer);
    freeResponse(res);
}

int findEmptyRoomId()
{
    int roomid;
    for (int i = 0; i < MAXROOM; i++)
    {
        if (roomIds[i] == 0)
        {
            return i;
        }
    }
}