#include "room.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void clearRoomList(Room *head)
{
    Room *tmp;
    Room *tmp2;
    // printf("Clearing list...\n");
    tmp = head;
    while (tmp != NULL)
    {
        tmp2 = tmp;
        tmp = tmp->next;
        free(tmp2);
    }
}

Room *searchRoomById(Room *head, int id)
{
    Room *result;

    result = head;

    while (result != NULL)
    {
        if (id == result->id)
        {
            return result;
        }
        else
        {
            result = result->next;
        }
    }

    return NULL;
}

Room *addRoom(Room *head, int id, int ownerFd)
{
    Room *toBeAdded = (Room *)malloc(sizeof(Room));
    //assign value to new room
    toBeAdded->id = id;
    for (int i = 0; i < 4; i++)
    {
        toBeAdded->clientFd[i] = 0;
        toBeAdded->ready[i] = false;
    }
    addClientToRoom(toBeAdded, ownerFd);

    Room *current = head;

    if (searchRoomById(head, id) != NULL)
    {
        printf("ERR: Room with id: %d aleady existed\n", id);
    }

    if (head == NULL)
    {
        head = toBeAdded;
        return head;
    }
    else
    {
        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = toBeAdded;
        toBeAdded->next = NULL;
        return toBeAdded;
    }
}

Room *quickJoinRoom(Room *head, int backupRoomId, int ownerFd)
{
    Room *current = head;
    while (current != NULL)
    {
        if (addClientToRoom(current, ownerFd))
        {
            return current;
        }
        current = current->next;
    }
    return addRoom(head, backupRoomId, ownerFd);
}

bool removeRoom(Room *head, int id)
{
    Room *toBeRemoved = searchRoomById(head, id);

    Room *current = head;
    Room *prev = head;

    if (toBeRemoved == NULL)
    {
        printf("ERR: No room with id: %d to be removed\n", id);
        return false;
    }
    else
    {
        while (current->id != id)
        {
            prev = current;
            current = current->next;
        }

        prev->next = current->next;
        current->next = NULL;

        free(current);
        return true;
    }
}

bool addClientToRoom(Room *room, int clientFd)
{
    if (room != NULL)
    {
        for (int i = 0; i < 4; i++)
        {
            if (room->clientFd[i] == clientFd)
            {
                printf("ERR: Client is already in this room\n");
                return false;
            }
        }

        for (int i = 0; i < 4; i++)
        {
            if (room->clientFd[i] == 0)
            {
                room->clientFd[i] = clientFd;
                return true;
            }
        }

        printf("ERR: This room is full\n");
        return false;
    }
    printf("ERR: No room\n");
    return false;
}

bool removeClientFromRoom(Room *room, int clientFd)
{
    if (room != NULL)
    {
        for (int i = 0; i < 4; i++)
        {
            if (room->clientFd[i] == clientFd)
            {
                room->clientFd[i] = 0;
                room->ready[i] = false;
                return true;
            }
        }
        printf("ERR: Client is not a member of this room\n");
        return false;
    }
    printf("ERR: No room\n");
    return false;
}

bool updateReadyStatus(Room *room, int clientFd)
{
    if (room != NULL)
    {
        for (int i = 0; i < 4; i++)
        {
            if (room->clientFd[i] == clientFd)
            {
                room->ready[i] = true;
                return true;
            }
        }
    }
    return false;
}

int calculateNumberOfClientInRoom(Room *room)
{
    int count = 0;
    if (room != NULL)
    {
        for (int i = 0; i < 4; i++)
        {
            if (room->clientFd[i] != 0)
            {
                count++;
            }
        }
    }

    return count;
}

int calculateNumberOfReadiedClient(Room *room)
{
    int count = 0;
    if (room != NULL)
    {
        for (int i = 0; i < 4; i++)
        {
            if (room->ready[i] == true)
            {
                // printf("i = %d\n", i);
                count++;
            }
        }
    }
    return count;
}
