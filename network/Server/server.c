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

void updateRoomStatus(Room *room);

void initGame(Room *room);

void move(int socketFd, Room *room, Move *move);

void quitGame(int socketFd, Room *room);

void invalid(int socketFd);

int findEmptyRoomId();

Room *head;

int roomIds[MAXROOM] = {0};

pthread_mutex_t roomListLock;
pthread_mutex_t roomIdsLock;

int main(int argc, char const *argv[])
{
    // server, client and bytes count
    int listenFd, connectFd;
    int portNumber = 0;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen;
    int opt = 1;

    head = (Room *)malloc(sizeof(Room));
    head->id = 0;

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

    if (pthread_mutex_init(&roomListLock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }

    if (pthread_mutex_init(&roomIdsLock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }

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
    unsigned char *outBuffer;
    int valread;
    Room *room = NULL;

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
            break;

        case QUICK_JOIN:
            room = quickJoin(socket, room);
            break;

        case JOIN_A_ROOM:;
            printf("%d\n", req->roomId);
            room = joinARoom(socket, room, req->roomId);
            break;

        case READY:
            ready(socket, room);

            int readyCount = calculateNumberOfReadiedClient(room);
            printf("READIED\n");

            if (readyCount == calculateNumberOfClientInRoom(room) && readyCount >= 2)
            {
                printf("READIED\n");

                initGame(room);
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
        pthread_mutex_lock(&roomListLock);
        int roomId = findEmptyRoomId();
        Room *room = addRoom(head, roomId, socketFd);
        pthread_mutex_unlock(&roomListLock);

        pthread_mutex_lock(&roomIdsLock);
        roomIds[roomId] = 1;
        pthread_mutex_unlock(&roomIdsLock);

        res->success = true;
        res->roomId = room->id;

        buffer = serializeResponse(res);

        send(socketFd, buffer, BUFFER_SIZE, 0);

        pthread_mutex_lock(&roomListLock);
        updateRoomStatus(room);
        pthread_mutex_unlock(&roomListLock);

        free(buffer);
        freeResponse(res);
        return room;
    }
    else
    {
        printf("CID: %d\n", curRoom->id);
        res->success = false;
        strcpy(res->err, "ALREADY IN ANOTHER ROOM");
        buffer = serializeResponse(res);

        send(socketFd, buffer, BUFFER_SIZE, 0);

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
        pthread_mutex_lock(&roomListLock);
        int roomId = findEmptyRoomId();
        Room *room = quickJoinRoom(head, roomId, socketFd);
        pthread_mutex_unlock(&roomListLock);

        if (room->id == roomId)
        {
            pthread_mutex_lock(&roomIdsLock);
            roomIds[roomId] = 1;
            pthread_mutex_unlock(&roomIdsLock);
        }

        res->success = true;
        res->roomId = room->id;

        buffer = serializeResponse(res);

        send(socketFd, buffer, BUFFER_SIZE, 0);

        pthread_mutex_lock(&roomListLock);
        updateRoomStatus(room);
        pthread_mutex_unlock(&roomListLock);

        free(buffer);
        freeResponse(res);
        return room;
    }
    else
    {
        res->success = false;
        strcpy(res->err, "ALREADY IN ANOTHER ROOM");
        buffer = serializeResponse(res);

        send(socketFd, buffer, BUFFER_SIZE, 0);

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
            if (calculateNumberOfClientInRoom(room) < 4)
            {
                pthread_mutex_lock(&roomListLock);
                addClientToRoom(room, socketFd);
                pthread_mutex_unlock(&roomListLock);

                res->success = true;
                res->roomId = room->id;

                buffer = serializeResponse(res);

                send(socketFd, buffer, BUFFER_SIZE, 0);

                pthread_mutex_lock(&roomListLock);
                updateRoomStatus(room);
                pthread_mutex_unlock(&roomListLock);

                return room;
            }
            else
            {
                res->success = false;
                strcpy(res->err, "ROOM IS FULL");
                buffer = serializeResponse(res);

                send(socketFd, buffer, BUFFER_SIZE, 0);

                return curRoom;
            }
        }
        else
        {
            res->success = false;
            strcpy(res->err, "ROOM NOT FOUND");
            buffer = serializeResponse(res);

            send(socketFd, buffer, BUFFER_SIZE, 0);
        }

        free(buffer);
        freeResponse(res);

        return curRoom;
    }
    else
    {
        res->success = false;
        strcpy(res->err, "ALREADY IN ANOTHER ROOM");

        buffer = serializeResponse(res);

        send(socketFd, buffer, BUFFER_SIZE, 0);

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
        pthread_mutex_lock(&roomListLock);
        if (updateReadyStatus(room, socketFd))
        {
            res->success = true;
        }
        else
        {
            res->success = false;
            strcpy(res->err, "UPDATE FAILED");
        }
        pthread_mutex_unlock(&roomListLock);
    }
    else
    {
        res->success = false;
        strcpy(res->err, "YOU ARE NOT IN A ROOM");
    }

    buffer = serializeResponse(res);

    send(socketFd, buffer, BUFFER_SIZE, 0);

    updateRoomStatus(room);

    free(buffer);
    freeResponse(res);
}

void quitGame(int socketFd, Room *room)
{
    unsigned char *buffer;
    Response *res = (Response *)malloc(sizeof(Response));
    res->type = QUIT_GAME;

    if (room != NULL)
    {
        int quit = -1;

        for (int i = 0; i < 4; i++)
        {
            if (room->clientFd[i] == socketFd)
            {
                quit = i;
                pthread_mutex_lock(&roomListLock);
                removeClientFromRoom(room, socketFd);
                pthread_mutex_unlock(&roomListLock);
            }
        }

        if (quit != -1)
        {
            res->success = true;
            res->quitted = quit;

            buffer = serializeResponse(res);

            for (int i = 0; i < 4; i++)
            {
                if (room->clientFd[i] != socketFd && room->clientFd[i] != 0)
                {
                    send(room->clientFd[i], buffer, BUFFER_SIZE, 0);
                }
            }

            pthread_mutex_lock(&roomListLock);
            updateRoomStatus(room);
            if (calculateNumberOfClientInRoom(room) <= 0)
            {
                roomIds[room->id] = 0;
                removeRoom(head, room->id);
            }
            pthread_mutex_unlock(&roomListLock);
        }
        else
        {
            res->success = false;
            strcpy(res->err, "YOU ARE NOT IN THIS ROOM");
            buffer = serializeResponse(res);
            send(socketFd, buffer, BUFFER_SIZE, 0);
        }
    }
    else
    {
        res->success = false;
        strcpy(res->err, "YOU ARE NOT IN A ROOM");
        buffer = serializeResponse(res);
        send(socketFd, buffer, BUFFER_SIZE, 0);
    }
    free(buffer);
}

void updateRoomStatus(Room *room)
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

        for (int i = 0; i < 4; i++)
        {
            if (room->clientFd[i] != 0)
            {
                send(room->clientFd[i], buffer, BUFFER_SIZE, 0);
            }
        }

        free(buffer);
        // freeResponse(res);
    }
}

void initGame(Room *room)
{
    unsigned char *buffer;

    for (int i = 0; i < 4; i++)
    {
        Response *res = (Response *)malloc(sizeof(Response));
        res->type = GAME_INIT;
        res->success = true;
        GameInitInfo *gi = (GameInitInfo *)malloc(sizeof(GameInitInfo));

        if (room->clientFd[i] != 0)
        {
            gi->yourColor = i;
            gi->playerCount = calculateNumberOfClientInRoom(room);
        }

        res->gameInitInfo = gi;

        buffer = serializeResponse(res);

        send(room->clientFd[i], buffer, BUFFER_SIZE, 0);

        // freeResponse(res);
        free(buffer);
    }
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
                send(room->clientFd[i], buffer, BUFFER_SIZE, 0);
            }
        }
    }
    else
    {
        res->success = false;
        strcpy(res->err, "INVALID MOVE");
        buffer = serializeResponse(res);
        send(socketFd, buffer, BUFFER_SIZE, 0);
    }
    free(buffer);
    // freeResponse(res);
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

    for (int i = 1; i < MAXROOM; i++)
    {
        if (roomIds[i] == 0)
        {
            return i;
        }
    }
}