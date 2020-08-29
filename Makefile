all: server

server: server.o net.o llist.o hashtable.o utils.o request.o response.o
	gcc -g -lpthread -o server server.o net.o llist.o hashtable.o utils.o request.o response.o

server.o: server.c
	gcc -c -g server.c

net.o: net.c
	gcc -c -g net.c

llist.o: llist.c
	gcc -c -g llist.c

hashtable.o: hashtable.c
	gcc -c -g hashtable.c

utils.o: utils.c
	gcc -c -g utils.c

request.o: request.c
	gcc -c -g request.c

response.o: response.c
	gcc -c -g response.c

clean:
	rm -f server.o net.o llist.o hashtable.o utils.o request.o response.o
	rm -f server

.PHONY: all
