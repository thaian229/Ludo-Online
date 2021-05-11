#include "deserialize.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdbool.h>

unsigned char *serialize_int(unsigned char *buffer, int value)
{
    // value = htonl(value);

    buffer[0] = value;
    buffer[1] = value >> 8;
    buffer[2] = value >> 16;
    buffer[3] = value >> 24;

    // printf("DI: %d\n",value);

    return buffer + 4;
}

unsigned char *serialize_bool(unsigned char *buffer, bool value)
{
    if (value == true)
    {
        buffer[0] = 0x01;
    }
    else
    {
        buffer[0] = 0x00;
    }
    return buffer + 1;
}

unsigned char *serialize_type(unsigned char *buffer, Type value)
{
    switch (value)
    {
    case CREATE_ROOM:
        buffer[0] = 0x01;
        break;

    case QUICK_JOIN:
        buffer[0] = 0x02;
        break;

    case JOIN_A_ROOM:
        buffer[0] = 0x03;
        break;

    case READY:
        buffer[0] = 0x04;
        break;

    case MOVE:
        buffer[0] = 0x05;
        break;

    case QUIT_GAME:
        buffer[0] = 0x06;
        break;

    case CREATE_ROOM_RESPONSE:
        buffer[0] = 0x10;
        break;

    case QUICK_JOIN_RESPONSE:
        buffer[0] = 0x20;
        break;

    case JOIN_A_ROOM_RESPONSE:
        buffer[0] = 0x30;
        break;

    case READY_RESPONSE:
        buffer[0] = 0x40;
        break;

    case ROOM_STATUS_UPDATE:
        buffer[0] = 0x50;
        break;

    case GAME_INIT:
        buffer[0] = 0x60;
        break;

    default:
        printf("ERR: INVALID TYPE\n");
        buffer[0] = 0x00;
        break;
    }
    return buffer + 1;
}

unsigned char *serialize_string(unsigned char *buffer, char *value)
{
    while (*value != 0)
    {
        *buffer = *value;
        buffer++;
        value++;
    }
    *buffer = 0;

    return buffer + 1;
}
