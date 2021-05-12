#include "deserialize.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

Request *deserializeRequest(unsigned char *buffer)
{
    Request *req = (Request *)malloc(sizeof(Request));

    buffer = deserialize_type(buffer, &req->type);

    switch (req->type)
    {
    case CREATE_ROOM:
    case QUICK_JOIN:
    case READY:
    case QUIT_GAME:
        break;

    case JOIN_A_ROOM:
        buffer = deserialize_int(buffer, &req->roomId);
        break;

    case MOVE:;
        Move *move = (Move *)malloc(sizeof(Move));
        req->move = move;
        buffer = deserialize_int(buffer, &req->move->turn);
        buffer = deserialize_int(buffer, &req->move->pieceNo);
        buffer = deserialize_int(buffer, &req->move->diceValue);
        break;

    default:
        printf("ERR: CAN'T GET TYPE FROM MESSAGE\n");
        req->type = INVALID;
        break;
    }
    
    return req;
}


void freeRequest(Request *req)
{
    if (req != NULL)
    {
        switch (req->type)
        {
        case MOVE:
            free(req->move);
            free(req);
            break;

        default:
            free(req);
            break;
        }
    }
}