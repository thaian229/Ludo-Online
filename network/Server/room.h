#ifndef __ROOM_H__
#define __ROOM_H__
#include <stdbool.h>

typedef struct Room
{
    int id;
    int clientFd[4];
    bool ready[4];
    struct Room *next;
} Room;

void clearRoomList(Room *head);

Room *searchRoomById(Room *head, int id);

Room *addRoom(Room *head, int id, int ownerFd);

Room *quickJoinRoom(Room *head, int backupRoomId, int ownerFd);

bool removeRoom(Room *head, int id);

bool removeClientFromRoom(Room *room, int clientFd);

bool addClientToRoom(Room *room, int clientFd);

bool updateReadyStatus(Room *room, int clientFd);

#endif
