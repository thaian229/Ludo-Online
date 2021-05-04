// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>
#include "response.h"
#include "request.h"

#define BUFFER_SIZE 512
#define PORT 5500
#define IP "127.0.0.1"

int socketFd, valread;

struct sockaddr_in serverAddr;

int portNumber;

unsigned char inBuffer[BUFFER_SIZE] = {0};

unsigned char outBuffer[BUFFER_SIZE] = {0};

pthread_t threads[2] = {0};

//message
bool res_success;

//room id
int ri_roomId;

//move
int m_moveX;
int m_moveY;
int m_diceValue;
int m_turn;

//room status
int rs_players;
int rs_ready;
bool gameStarted = false;

//game init info
int gif_yourColor;
int gif_playerCount;

// void *send_handler(void *);

void *recv_handler(void *);


void handle_room_status_update(RoomStatus *roomStatus);
void handle_game_init(GameInitInfo *gif);
void handle_move(Move *move);
void handle_room_create(int roomId);
void handle_quick_join(int roomId);
void handle_join_a_room(int roomId);
int close_connection();
int connect_to_server();

int close_connection()
{
    if (threads[0] != 0)
    {
        pthread_join(threads[0], NULL);
    }

    // if (threads[1] != 0)
    // {
    //     pthread_join(threads[1], NULL);
    // }

    if (socketFd != 0)
    {
        close(socketFd);
        return 1;
    }
    else
    {
        return 0;
    }
}

int connect_to_server()
{
    // Creating socket file descriptor
    if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        return 0;
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
        return 0;
    }

    if (pthread_create(&threads[0], NULL, recv_handler, &socketFd) < 0)
    {
        perror("Could not create thread");
        close_connection();
        return 0;
    }

    printf("Connected to the server...\n");

    return socketFd;
}

void *recv_handler(void *socketFd)
{
    int socket = *(int *)socketFd;
    int valread;
    while (valread = read(socket, inBuffer, sizeof(inBuffer)))
    {
        Response *res = deserializeResponse(inBuffer);

        if (res->success)
        {
            switch (res->type)
            {
            case ROOM_STATUS_UPDATE:
                handle_room_status_update(res->roomStatus);
                break;

            case GAME_INIT:
                handle_game_init(res->gameInitInfo);
                break;

            case MOVE:
                handle_move(res->move);
                break;

            case CREATE_ROOM_RESPONSE:
                handle_room_create(res->roomId);
                break;

            case QUICK_JOIN_RESPONSE:
                handle_quick_join(res->roomId);
                break;

            case JOIN_A_ROOM_RESPONSE:
                handle_join_a_room(res->roomId);
                break;

            case READY_RESPONSE:
                break;

            default:
                // printf("Error: %s\n", "INVALID TYPE");
                break;
            }
        }
        else
        {
            printf("Error: %s\n", res->err);
        }

        freeResponse(res);
        memset(inBuffer, 0, sizeof(inBuffer));
    }
}



void handle_room_status_update(RoomStatus *roomStatus)
{
    rs_players = roomStatus->players;
    rs_ready = roomStatus->ready;
}

void handle_game_init(GameInitInfo *gif)
{
    gif_playerCount = gif->playerCount;
    gif_yourColor = gif->yourColor;
    gameStarted = true;
}

void handle_move(Move *move)
{
    m_diceValue = move->diceValue;
    m_moveX = move->moveX;
    m_moveY = move->moveY;
    m_turn = move->turn;
}

void handle_room_create(int roomId)
{
    ri_roomId = roomId;
}

void handle_quick_join(int roomId)
{
    ri_roomId = roomId;
}

void handle_join_a_room(int roomId)
{
    ri_roomId = roomId;
}

//     case CREATE_ROOM:
void send_create_room()
{
    Request *req = (Request *)malloc(sizeof(Request));

    req->type = CREATE_ROOM;

    strcpy(outBuffer, serializeRequest(req));

    send(socketFd, outBuffer, sizeof(outBuffer), 0);

    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}

//     case QUICK_JOIN:
void send_quick_join()
{
    Request *req = (Request *)malloc(sizeof(Request));

    req->type = QUICK_JOIN;

    strcpy(outBuffer, serializeRequest(req));

    send(socketFd, outBuffer, sizeof(outBuffer), 0);

    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}

//     case READY:
void send_ready()
{
    Request *req = (Request *)malloc(sizeof(Request));

    req->type = READY;

    strcpy(outBuffer, serializeRequest(req));

    send(socketFd, outBuffer, sizeof(outBuffer), 0);

    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}

//     case QUIT_GAME:
void send_quit_game()
{
    Request *req = (Request *)malloc(sizeof(Request));

    req->type = QUIT_GAME;

    strcpy(outBuffer, serializeRequest(req));

    send(socketFd, outBuffer, sizeof(outBuffer), 0);

    close_connection();

    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}

//     case JOIN_A_ROOM:
void send_join_a_room(int id)
{
    Request *req = (Request *)malloc(sizeof(Request));

    req->type = JOIN_A_ROOM;
    req->roomId = id;

    strcpy(outBuffer, serializeRequest(req));

    send(socketFd, outBuffer, sizeof(outBuffer), 0);

    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}

//     case MOVE:
void send_move(int turn, int dice, int moveX, int moveY)
{
    Request *req = (Request *)malloc(sizeof(Request));
    Move *move = (Move *)malloc(sizeof(Move));

    move->turn = turn;
    move->diceValue = dice;
    move->moveX = moveX;
    move->moveY = moveY;

    req->type = CREATE_ROOM;
    req->move = move;

    strcpy(outBuffer, serializeRequest(req));

    send(socketFd, outBuffer, sizeof(outBuffer), 0);

    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}