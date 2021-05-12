#include "serialize.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

unsigned char *serializeRequest(Request *req)
{
    unsigned char *buffer = (unsigned char *)malloc(512);

    unsigned char *ptr;

    ptr = buffer;

    ptr = serialize_type(ptr, req->type);

    switch (req->type)
    {
    case CREATE_ROOM:
    case QUICK_JOIN:
    case READY:
    case QUIT_GAME:
        break;

    case JOIN_A_ROOM:
        ptr = serialize_int(ptr, req->roomId);
        printf("%d\n", req->roomId);
        break;

    case MOVE:
        ptr = serialize_int(ptr, req->move->turn);
        ptr = serialize_int(ptr, req->move->pieceNo);
        ptr = serialize_int(ptr, req->move->diceValue);
        break;

    default:
        printf("ERR: CAN'T GET TYPE FROM MESSAGE\n");
        req->type = INVALID;
        break;
    }
    return buffer;
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