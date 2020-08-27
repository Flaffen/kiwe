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

int get_http_line(char *request, char *line, size_t *curp)
{
	int i = 0;
	size_t cur = *curp;

	while (1) {
		if (cur == -1) return -1;
		if (cur > strlen(request)) return -1;

		line[i] = request[cur];

		i++;
		cur++;

		if (request[cur] == '\r') {
			if (request[cur + 1] == '\n') {
				cur++;
				cur++;
			} else if (request[cur + 1] == '\r') {
				cur = -1;
			} else {
				cur++;
			}

			return 0;
		} else if (request[cur] == '\n') {
			if (request[cur + 1] == '\n') {
				cur = -1;
			} else {
				cur++;
			}

			return 0;
		}
	}
}

int consume_http_line(char **request, char *line)
{
	char *cur = *request;

	if (*cur == '\r')
		return -1;

	while (1) {
		strncat(line, cur, 1);
		cur++;

		if (*cur == '\r') {
			if (*(cur + 1) == '\n') {
				cur += 2;
				*request = cur;
				return 0;
			} else if (*(cur + 1) == '\r') {
				*request = cur;
				cur  += 2;
				return -1;
			} else {
				*request = cur;
				cur += 1;
				return 0;
			}
		} else if (*cur == '\n') {
			if (*(cur + 1) == '\n') {
				*request = cur;
				cur += 2;
				return -1;
			} else {
				*request = cur;
				cur += 1;
				return 0;
			}
		}
	}
}

struct request_info {
	struct sockaddr_storage their_addr;
	int sockfd;
	// struct cache *cache;
};

struct request_info *create_request_info(struct sockaddr_storage their_addr, int sockfd) {
	struct request_info *re = malloc(sizeof(*re));

	re->their_addr = their_addr;
	re->sockfd = sockfd;

	return re;
}

struct llist *get_request_headers(char *request)
{
	char line[512] = {0};
	struct hashtable *ht = hashtable_create(0, NULL);
	struct llist *list = llist_create(); 

	while (consume_http_line(&request, line) != -1) {
		char key[64] = {0};
		char value[512] = {0};
		sscanf(line, "%s %s", key, value);
		key[strlen(key) - 1] = '\0';

		char htkey[strlen(key) + 1];
		strcpy(htkey, key);
		char *htvalue = malloc(strlen(value) + 1);
		strcpy(htvalue, value);

		char *llist_entry[2];
		llist_entry[0] = htkey;
		llist_entry[1] = htvalue;
		llist_append(list, (void *) llist_entry);

		void *voided = (void *) llist_entry;

		char (*returnedlist)[2] = (char (*)[2]) voided;

		memset(line, 0, 512);
	}

	return list;
}

struct response {
	struct llist *headers;
	void *data;
};

void print_ht_entry(void *key, void *value)
{
	printf("");
}

struct response *get_response(char *method, char *path, struct llist *req_headers)
{
	struct response *resp = malloc(sizeof(*resp));
	struct llist *resp_headers = llist_create();

	if (strcmp(path, "hello") == 0) {
		char *response_string = "hello";
		resp->data = response_string;
	}

	return resp;
}

void print_each_element(void *data, void *arg)
{
	char (*header)[2] = (char (*)[2]) data;
	printf("%s %s\n", header[0], header[1]);
}

char *get_headers(struct llist *headers)
{
	// DOES NOT WORK
	char (*header)[2] = (char (*)[2]) headers->head;
	printf("%s\n", header[0]);
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
	char *first = request;
	recv(newfd, request, 65536, 0);

	char first_line[512] = {0};
	consume_http_line(&first, first_line);
	char method[16], path[256];
	sscanf(first_line, "%s %s", method, path);

	struct llist *req_headers = get_request_headers(request);

	//struct response *resp = get_response(method, path, req_headers);
	get_headers(req_headers);

	char *respstr = "HTTP/1.1 404 NOT FOUND";
	send(newfd, respstr, strlen(respstr), 0);

	printf("%s:%d disconnected\n", s, ((struct sockaddr_in *) &their_addr)->sin_port);

	free(re);
	llist_destroy(req_headers);
	close(newfd);

	pthread_exit((void *) 0);
}

int main(int argc, char *argv[])
{
	int listenfd, newfd;
	struct sockaddr_storage their_addr;
	char *port = "4590";
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
}