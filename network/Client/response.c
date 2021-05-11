#include "response.h"
#include "deserialize.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

Response *deserializeResponse(unsigned char *buffer)
{
    Response *res = (Response *)malloc(sizeof(Response));

    buffer = deserialize_bool(buffer, &res->success);

    buffer = deserialize_type(buffer, &res->type);

    if (!res->success)
    {
        buffer = deserialize_string(buffer, res->err);
        return res;
    }

    switch (res->type)
    {
    case ROOM_STATUS_UPDATE:;
        RoomStatus *rs = (RoomStatus *)malloc(sizeof(RoomStatus));
        res->roomStatus = rs;

        buffer = deserialize_int(buffer, &res->roomStatus->players);
        buffer = deserialize_int(buffer, &res->roomStatus->ready);
        break;

    case GAME_INIT:;
        GameInitInfo *gif = (GameInitInfo *)malloc(sizeof(GameInitInfo));
        res->gameInitInfo = gif;

        buffer = deserialize_int(buffer, &res->gameInitInfo->yourColor);
        buffer = deserialize_int(buffer, &res->gameInitInfo->playerCount);
        break;

    case MOVE:;
        Move *move = (Move *)malloc(sizeof(Move));
        res->move = move;
        buffer = deserialize_int(buffer, &res->move->turn);
        buffer = deserialize_int(buffer, &res->move->diceValue);
        buffer = deserialize_int(buffer, &res->move->moveX);
        buffer = deserialize_int(buffer, &res->move->moveY);
        break;

    case CREATE_ROOM_RESPONSE:
    case QUICK_JOIN_RESPONSE:
    case JOIN_A_ROOM_RESPONSE:
        buffer = deserialize_int(buffer, &res->roomId);
        printf("roomid: %d\n", res->roomId);
        printf("here\n");
        break;

    case READY_RESPONSE:
        break;

    default:
        printf("ERR: CAN'T GET TYPE FROM MESSAGE\n");
        break;
    }

    return res;
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