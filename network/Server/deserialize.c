#include "deserialize.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

unsigned char *deserialize_int(unsigned char *buffer, int *value)
{
    unsigned int tmp = (buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24);
    // unsigned int tmp = (buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3] << 0);
    // printf("%d\n", tmp);
    // *value = ntohl(tmp);
    *value = tmp;
    return buffer + 4;
}

unsigned char *deserialize_bool(unsigned char *buffer, bool *value)
{
    if (buffer[0] == 0x00)
    {
        *value = false;
    }
    else
    {
        *value = true;
    }
    return buffer + 1;
}

unsigned char *deserialize_type(unsigned char *buffer, Type *value)
{
    unsigned char type;
    type = buffer[0];

    switch (type)
    {
    case 0x01:
        *value = CREATE_ROOM;
        break;

    case 0x02:
        *value = QUICK_JOIN;
        break;

    case 0x03:
        *value = JOIN_A_ROOM;
        break;

    case 0x04:
        *value = READY;
        break;

    case 0x05:
        *value = MOVE;
        break;

    case 0x06:
        *value = QUIT_GAME;
        break;

    case 0x10:
        *value = CREATE_ROOM_RESPONSE;
        break;

    case 0x20:
        *value = QUICK_JOIN_RESPONSE;
        break;

    case 0x30:
        *value = JOIN_A_ROOM_RESPONSE;
        break;

    case 0x40:
        *value = READY_RESPONSE;
        break;

    case 0x50:
        *value = ROOM_STATUS_UPDATE;
        break;

    case 0x60:
        *value = GAME_INIT;
        break;

    default:
        *value = INVALID;
        break;
    }
    return buffer + 1;
}

unsigned char *deserialize_string(unsigned char *buffer, char *value)
{
    char *tmp = value;

    while (*buffer != 0)
    {
        *tmp = *buffer;
        buffer++;
        tmp++;
    }


    return buffer + 1;
}
