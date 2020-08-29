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
	strcat(response_data, resp->data);
}

void *handle_http_request(void *data)
{
	struct request_info *re = (struct request_info *) data;
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr = re->their_addr;
	int newfd = re->sockfd;

	inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof(s));
	printf("%s:%d connected\n", s, ((struct sockaddr_in *) &their_addr)->sin_port);

	char request[65536];
	memset(request, 0, 65536);
	
	while (recv(newfd, request, 65536, 0) > 0) {
		char *first = request;

		char first_line[512] = {0};
		char method[16], path[256];

		consume_http_line(&first, first_line);
		sscanf(first_line, "%s %s", method, path);

		struct llist *req_headers = get_request_headers(request);
		struct response *resp = get_response(method, path, req_headers);

		int content_length = get_content_length(resp->headers);
		size_t max_response_length = content_length + 256000;
		char *response_data = malloc(max_response_length);
		assemble_response_data(resp, response_data);

		printf("%s", response_data);

		send(newfd, response_data, max_response_length,0);

		llist_destroy(req_headers);
		free_response(resp);
		free(response_data);
		memset(request, 0, 65536);
	}

	printf("%s:%d disconnected\n", s, ((struct sockaddr_in *) &their_addr)->sin_port);

	free(re);
	close(newfd);

	pthread_exit((void *) 0);
}

int main(int argc, char *argv[])
{
	int listenfd, newfd;
	struct sockaddr_storage their_addr;
	char *port = "3490";
	pthread_t threadid = 0;

	if (argc > 1) {
		port = argv[1];
	}

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
			// perror("accept");
			continue;
		}

		struct request_info *re = create_request_info(their_addr, newfd);

		pthread_create(&threadid, NULL, handle_http_request, (void *) re);
		pthread_detach(threadid);
		threadid++;
	}

	return 0;
}
