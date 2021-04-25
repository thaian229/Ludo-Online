CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 


all: client

# client: client.c utils.o serialize.o deserialize.o message.o
# 	${CC} -pthread client.c utils.o serialize.o deserialize.o message.o -o client 

client: client.c
	${CC} -pthread client.c -o client 

serialize.o: serialize.c
	${CC} ${CFLAGS} serialize.c	
	
deserialize.o: deserialize.c
	${CC} ${CFLAGS} deserialize.c	

message.o: message.c
	${CC} ${CFLAGS} message.c

clean:
	rm -f client *.o