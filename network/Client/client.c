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

pthread_t threads[2] = {0};

//message
bool flag_res_failed = false;

//room id
int ri_roomId = 0;

//move
int m_diceValue = 1;
int m_piece_no;
int m_turn;
bool m_move_info_ready = false;

//quit
int q_quiter = -1;
bool q_quit_event_ready = false;

//room status
int rs_players = 0;
int rs_ready = 0;
bool gameStarted = false;

//game init info
int gif_yourColor;
int gif_playerCount;

// void *send_handler(void *);

void *recv_handler(void *);

void handle_room_status_update(RoomStatus *roomStatus);
void handle_game_init(GameInitInfo *gif);
void handle_move(Move *move);
void handle_quit(int quitted);
void handle_room_create(int roomId);
void handle_quick_join(int roomId);
void handle_join_a_room(int roomId);
int close_connection();
int connect_to_server();

int close_connection()
{
    // if (threads[0] != 0)
    // {
    //     pthread_join(threads[0], NULL);
    // }

    // if (threads[1] != 0)
    // {
    //     pthread_join(threads[1], NULL);
    // }

    if (socketFd != 0)
    {
        close(socketFd);
        socketFd = 0;
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
    if (socketFd != 0)
    {
        return socketFd;
    }

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
        if (valread >= 0)
        {
            printf("\n============================\n\n");
            for (int i = 0; i < valread; i++)
            {
                printf("%X", inBuffer[i]);
            }
            printf("\n\n============================\n\n");

            Response *res = deserializeResponse(inBuffer);

            if (res->success)
            {
                switch (res->type)
                {
                case ROOM_STATUS_UPDATE:
                    // printf("UPDATE\n");
                    handle_room_status_update(res->roomStatus);
                    break;

                case GAME_INIT:
                    // printf("INIT\n");
                    handle_game_init(res->gameInitInfo);
                    break;

                case MOVE:
                    // printf("MOVE\n");
                    handle_move(res->move);
                    break;

                case QUIT_GAME:
                    handle_quit(res->quitted);
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
                flag_res_failed = true;
            }

            // freeResponse(res);
            memset(inBuffer, 0, sizeof(inBuffer));
        }
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
    m_piece_no = move->pieceNo;
    m_turn = move->turn;
    m_move_info_ready = true;
}

void handle_quit(int quitted)
{
    q_quiter = quitted;
    q_quit_event_ready = true;
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

    unsigned char *outBuffer;

    outBuffer = serializeRequest(req);

    send(socketFd, outBuffer, BUFFER_SIZE, 0);

    free(outBuffer);

    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}

//     case QUICK_JOIN:
void send_quick_join()
{
    Request *req = (Request *)malloc(sizeof(Request));

    req->type = QUICK_JOIN;

    unsigned char *outBuffer;

    outBuffer = serializeRequest(req);

    send(socketFd, outBuffer, BUFFER_SIZE, 0);

    free(outBuffer);

    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}

//     case READY:
void send_ready()
{
    Request *req = (Request *)malloc(sizeof(Request));

    req->type = READY;

    unsigned char *outBuffer;

    outBuffer = serializeRequest(req);
    send(socketFd, outBuffer, BUFFER_SIZE, 0);

    free(outBuffer);

    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}

//     case QUIT_GAME:
void send_quit_game()
{
    Request *req = (Request *)malloc(sizeof(Request));

    req->type = QUIT_GAME;

    unsigned char *outBuffer;

    outBuffer = serializeRequest(req);

    send(socketFd, outBuffer, BUFFER_SIZE, 0);

    // close_connection();

    free(outBuffer);
    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}

//     case JOIN_A_ROOM:
void send_join_a_room(int id)
{
    Request *req = (Request *)malloc(sizeof(Request));

    req->type = JOIN_A_ROOM;
    req->roomId = id;

    unsigned char *outBuffer;

    outBuffer = serializeRequest(req);

    send(socketFd, outBuffer, BUFFER_SIZE, 0);

    free(outBuffer);

    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}

//     case MOVE:
void send_move(int turn, int pieceNo, int dice)
{
    Request *req = (Request *)malloc(sizeof(Request));
    Move *move = (Move *)malloc(sizeof(Move));

    move->turn = turn;
    move->pieceNo = pieceNo;
    move->diceValue = dice;

    req->type = MOVE;
    req->move = move;

    unsigned char *outBuffer;

    outBuffer = serializeRequest(req);

    send(socketFd, outBuffer, BUFFER_SIZE, 0);

    free(outBuffer);

    freeRequest(req);
    memset(inBuffer, 0, sizeof(inBuffer));
}

void main()
{
    connect_to_server();
    // send_join_a_room(1);

    send_quick_join();
    // printf("done\n");

    send_create_room();

    send_ready();

    send_quit_game();

    while (1)
    {
        /* code */
    }
}

int get_room_id()
{
    return ri_roomId;
}

int get_rs_players()
{
    return rs_players;
}

int get_rs_ready()
{
    return rs_ready;
}

int get_game_started()
{
    if (gameStarted == false)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int get_gif_color()
{
    return gif_yourColor;
}

int get_gif_player_count()
{
    return gif_playerCount;
}

int get_piece_no()
{
    return m_piece_no;
}

int get_turn()
{
    return m_turn;
}

int get_dice_value()
{
    return m_diceValue;
}

int get_quiter()
{
    return q_quiter;
}

int get_move_ready()
{
    if (m_move_info_ready == false)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void set_move_ready(int i)
{
    if (i == 0)
    {
        m_move_info_ready = false;
    }
    else if (i == 1)
    {
        m_move_info_ready = true;
    }
}

int get_quit_event_ready()
{
    if (q_quit_event_ready == false)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void set_quit_event_ready(int i)
{
    if (i == 0)
    {
        q_quit_event_ready = false;
    }
    else if (i == 1)
    {
        q_quit_event_ready = true;
    }
}

int get_flag_res_failed()
{
    if (flag_res_failed == false)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void set_flag_res_failed(int i)
{
    if (i == 0)
    {
        flag_res_failed = false;
    }
    else if (i == 1)
    {
        flag_res_failed = true;
    }
}

void reset_game_info()
{
    m_diceValue = 0;
    m_move_info_ready = false;
    m_piece_no = 0;
    m_turn = 0;

    ri_roomId = 0;
    rs_players = 0;
    rs_ready = 0;
    gameStarted = false;

    gif_playerCount = 0;
    gif_yourColor = 0;

    q_quiter = 0;
    q_quit_event_ready = false;
}