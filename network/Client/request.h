#ifndef __REQUEST_H__
#define __REQUEST_H__

typedef enum Type
{
    CREATE_ROOM,
    QUICK_JOIN,
    JOIN_A_ROOM,
    READY,
    QUIT_GAME,

    MOVE,

    CREATE_ROOM_RESPONSE,
    QUICK_JOIN_RESPONSE,
    JOIN_A_ROOM_RESPONSE,
    READY_RESPONSE,
    ROOM_STATUS_UPDATE,
    GAME_INIT,

    INVALID
} Type;

typedef struct Move
{
    int diceValue;
    int pieceNo;
    int turn;

} Move;

typedef struct Request
{
    Type type;
    union
    {
        char content[200];
        int quitted;
        int roomId;
        Move *move;
    };
} Request;

unsigned char *serializeRequest(Request *req);

void freeRequest(Request *req);

#endif
