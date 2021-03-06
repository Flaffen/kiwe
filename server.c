#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include <pthread.h>
#include "net.h"
#include "llist.h"
#include "hashtable.h"
#include "utils.h"
#include "request.h"
#include "response.h"
#include "cache.h"

#define PORT "3490"

pthread_mutex_t cachemutex;

void *handle_http_request(void *data)
{
	struct request_info *re = (struct request_info *) data;
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr = re->their_addr;
	int newfd = re->sockfd;
	struct cache *cache = re->cache;
	int rcount = 0;

	inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof(s));
	printf("[socket %d] %s:%d connected\n", newfd, s, ((struct sockaddr_in *) &their_addr)->sin_port);

	char request[65536];

	while (recv(newfd, request, 65536, 0) > 0) {
		rcount++;
		printf("curren rcount is %d\n", rcount);
		char *first = request;
		char first_line[512] = {0};
		char method[16], path[256];
		struct response resp;

		consume_http_line(&first, first_line);
		sscanf(first_line, "%s %s", method, path);

		printf("%s\n", first_line);

		struct llist *req_headers = get_request_headers(request);
		get_response(&resp, method, path, req_headers, cache);

		// Changing this variable affects how many HTTP requests the function will process before closing the TCP connection. Weird.
		// 4kB for headers and stuff and rest is payload length.
		size_t max_response_length = resp.data_len + 4096;

		char *response_data = malloc(max_response_length);

		memset(response_data, 0, max_response_length);

		assemble_response_data(&resp, response_data);

		print_headers(resp.headers);

		send(newfd, response_data, max_response_length, 0);

		llist_destroy(req_headers);
		// free_response(resp);
		free(response_data);

		memset(request, 0, 65536);
		cache_print(cache);
	}

	free(re);

	printf("%s:%d disconnected, requests served on one connection - %d\n", s, ((struct sockaddr_in *) &their_addr)->sin_port, rcount);

	close(newfd);
	pthread_exit((void *) 0);
}

/* This function works just fine */
void *testfunc(void *data)
{
	struct request_info *re = (struct request_info *) data;
	int newfd = re->sockfd;
	int rcount = 0;

	printf("%d\n", newfd);

	char request[65536];
	while (recv(newfd, request, 65536, 0) > 0) {
		rcount++;
		printf("%s", request);
		char *str = "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">";
		char response[512] = {0};
		strcat(response, "HTTP/1.1 200 OK\nContent-Type: text/html\n");
		char content_length[64] = {0};
		sprintf(content_length, "%d", strlen(str));
		strcat(response, "Content-Length: ");
		strcat(response, content_length);
		strcat(response, "\n\n");
		strcat(response, str);

		printf("%s\n", response);

		send(newfd, response, strlen(response), 0);
		memset(request, 0, 65536);
	}

	printf("closed %d\n", rcount);
	close(newfd);

	pthread_exit((void *) 0);
}

int main(int argc, char *argv[])
{
	int listenfd, newfd;
	struct sockaddr_storage their_addr;
	char *port = argc > 1 ? argv[1] : PORT;
	pthread_mutex_init(&cachemutex, NULL);
	struct cache *cache = cache_create(50, 0, &cachemutex);
	pthread_t threadid;

	listenfd = get_listener_socket(port);
	if (listenfd < 0) {
		fprintf(stderr, "server: could not get listening socket\n");
		exit(1);
	}

	printf("waiting for connections on port %s\n", port);

	while (1) {
		socklen_t sin_size = sizeof(their_addr);

		newfd = accept(listenfd, (struct sockaddr *) &their_addr, &sin_size);
		if (newfd == -1) {
			perror("accept");
			continue;
		}

		struct request_info *re = create_request_info(their_addr, newfd, cache);

		pthread_create(&threadid, NULL, handle_http_request, (void *) re);
		pthread_detach(threadid);
	}

	return 0;
}
