#ifndef __RESPONSE_H__
#define __RESPONSE_H__
#include "request.h"
#include <stdbool.h>

typedef struct GameInitInfo
{
    int yourColor;
    int playerCount;
} GameInitInfo;

typedef struct RoomStatus
{
    int players;
    int ready;
} RoomStatus;

typedef struct Response
{
    bool success;
    Type type;
    union
    {
        char err[200];
        int roomId;
        RoomStatus *roomStatus;
        GameInitInfo *gameInitInfo;
        Move *move;
    };
} Response;

Response *deserializeResponse(unsigned char *buffer);

void freeResponse(Response *res);

#endif
