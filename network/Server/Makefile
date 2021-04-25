CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 


all: clean server

server: server.c utils.o serialize.o deserialize.o request.o response.o room.o
	${CC} -pthread server.c utils.o serialize.o deserialize.o request.o response.o room.o -o server 

utils.o: utils.c
	${CC} ${CFLAGS} utils.c

serialize.o: serialize.c
	${CC} ${CFLAGS} serialize.c	
	
deserialize.o: deserialize.c
	${CC} ${CFLAGS} deserialize.c	

request.o: request.c
	${CC} ${CFLAGS} request.c

response.o: response.c
	${CC} ${CFLAGS} response.c

room.o: room.c
	${CC} ${CFLAGS} room.c

clean:
	rm -f server *.o