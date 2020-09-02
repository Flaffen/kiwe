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

void append_header_to_response(void *data, void *arg)
{
	char *response_data = (char *) arg;
	char **header = (char **) data;
	char *key = header[0];
	char *value = header[1];
	char http_header[512] = {0};
	sprintf(http_header, "%s: %s\n", key, value);

	strcat(response_data, http_header);
}

void assemble_response_data(struct response *resp, char response_data[])
{
	strcat(response_data, resp->status);
	llist_foreach(resp->headers, append_header_to_response, response_data);

	strcat(response_data, "\n");
	char *startbody = strrchr(response_data, '\n') + 1;
	memcpy(startbody, resp->data, resp->data_len);
}

/*
 * This function should accept multiple HTTP requests but for some reason closes the connection after the first one, unlike testfunc(). The problem is in the while loop somewhere.
 */
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

		consume_http_line(&first, first_line);
		sscanf(first_line, "%s %s", method, path);

		printf("%s\n", first_line);

		struct llist *req_headers = get_request_headers(request);
		struct response *resp = get_response(method, path, req_headers, cache);

		size_t max_response_length = 32000 + resp->data_len;

		char *response_data = malloc(max_response_length);

		memset(response_data, 0, max_response_length);

		assemble_response_data(resp, response_data);

		print_headers(resp->headers);

		send(newfd, response_data, max_response_length, 0);

		llist_destroy(req_headers);
		free_response(resp);
		free(response_data);

		memset(request, 0, 65536);
		cache_print(cache);
	}

	free(re);

	printf("%s:%d disconnected, requests served on one connection - %d\n", s, ((struct sockaddr_in *) &their_addr)->sin_port, rcount);

	close(newfd);
	pthread_exit((void *) 0);
}

/**
 * This function accepts multiple HTTP requests before dropping the TCP connection due to timeout, which is the expected behavior.
 */
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
	struct cache *cache = cache_create(50, 0);
	pthread_t threadid = 0;

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
		threadid++;
	}

	return 0;
}
