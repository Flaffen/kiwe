all: server

server: server.o net.o llist.o hashtable.o utils.o request.o response.o file.o
	clang -g -lpthread -o server server.o net.o llist.o hashtable.o utils.o request.o response.o file.o

server.o: server.c
	clang -c -g server.c

net.o: net.c
	clang -c -g net.c

llist.o: llist.c
	clang -c -g llist.c

hashtable.o: hashtable.c
	clang -c -g hashtable.c

utils.o: utils.c
	clang -c -g utils.c

request.o: request.c
	clang -c -g request.c

response.o: response.c
	clang -c -g response.c

file.o: file.c
	clang -c -g file.c

clean:
	rm -f server.o net.o llist.o hashtable.o utils.o request.o response.o
	rm -f server

.PHONY: all
