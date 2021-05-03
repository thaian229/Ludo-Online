#include "response.h"
#include "serialize.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

unsigned char *serializeResponse(Response *res)
{
    unsigned char *buffer = (unsigned char *)malloc(512);

    unsigned char *ptr;

    ptr = buffer;

    ptr = serialize_bool(ptr, res->success);
    ptr = serialize_type(ptr, res->type);

    if(!res->success) {
        ptr = serialize_string(ptr, res->err);
        return buffer;
    }

    switch (res->type)
    {
    case ROOM_STATUS_UPDATE:
        ptr = serialize_int(ptr, res->roomStatus->players);
        ptr = serialize_int(ptr, res->roomStatus->ready);
        break;

    case GAME_INIT:
        ptr = serialize_int(ptr, res->gameInitInfo->yourColor);
        ptr = serialize_int(ptr, res->gameInitInfo->playerCount);
        break;

    case MOVE:
        ptr = serialize_int(ptr, res->move->turn);
        ptr = serialize_int(ptr, res->move->diceValue);
        ptr = serialize_int(ptr, res->move->moveX);
        ptr = serialize_int(ptr, res->move->moveY);
        break;

    case CREATE_ROOM_RESPONSE:
    case QUICK_JOIN_RESPONSE:
    case JOIN_A_ROOM_RESPONSE:
        ptr = serialize_int(ptr, res->roomId);
        break;

    case READY_RESPONSE:
        break;
    default:
        printf("ERR: CAN'T GET TYPE FROM MESSAGE\n");
        break;
    }
    return buffer;
}

void freeResponse(Response *res)
{
    if (res != NULL)
    {
        switch (res->type)
        {
        case MOVE:
            free(res->move);
            free(res);
            break;

        case ROOM_STATUS_UPDATE:
            free(res->roomStatus);
            free(res);

        case GAME_INIT:
            free(res->gameInitInfo);
            free(res);

        default:
            free(res);
            break;
        }
    }
}