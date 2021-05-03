CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 


all: clean client

client: client.c utils.o serialize.o deserialize.o request.o response.o
	${CC} -pthread client.c utils.o serialize.o deserialize.o request.o response.o -o client 

# client: client.c
# 	${CC} -pthread client.c -o client 
serialize.o: serialize.c
	${CC} ${CFLAGS} serialize.c	
	
deserialize.o: deserialize.c
	${CC} ${CFLAGS} deserialize.c	

request.o: request.c
	${CC} ${CFLAGS} request.c

response.o: response.c
	${CC} ${CFLAGS} response.c

clean:
	rm -f client *.o