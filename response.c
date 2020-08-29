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

struct response *get_response(char *method, char *path, struct llist *req_headers)
{
	struct response *resp = malloc(sizeof(*resp));
	struct llist *resp_headers = llist_create();
	resp->headers = resp_headers;

	if (strcmp(path, "/hello") == 0) {
		char *response_string = "hello";
		resp->data = response_string;
		char content_length[512];
		sprintf(content_length, "%d", strlen(response_string));
		llist_append(resp_headers, create_header("Content-Length", content_length));
		llist_append(resp_headers, create_header("Content-Type", "text/plain"));
	}

	return resp;
}
