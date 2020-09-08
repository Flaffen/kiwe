CC=clang
CFLAGS=-Wall -Wextra -g
all: server

OBJS=server.o net.o llist.o hashtable.o utils.o request.o response.o file.o mime.o cache.o

server: ${OBJS}
	clang -lpthread -o server $^

server.o: server.c

net.o: net.c

llist.o: llist.c

hashtable.o: hashtable.c

utils.o: utils.c

request.o: request.c

response.o: response.c

file.o: file.c

mime.o: mime.c

cache.o: cache.c

clean:
	rm -f server.o net.o llist.o hashtable.o utils.o request.o response.o
	rm -f server

.PHONY: all
