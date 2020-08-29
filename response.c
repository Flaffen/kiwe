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
#include "file.h"

struct response *get_response(char *method, char *path, struct llist *req_headers)
{
	struct response *resp = malloc(sizeof(*resp));
	struct llist *resp_headers = llist_create();
	char *status = malloc(sizeof(*status));
	status = "HTTP/1.1 404 NOT FOUND\n";
	resp->headers = resp_headers;
	if (strcmp(path, "/hello") == 0) {
		struct file_data *fdata = load_file("file.txt");
		char content_length[512];
		sprintf(content_length, "%d", fdata->len);
		llist_append(resp_headers, create_header("Content-Length", content_length));
		llist_append(resp_headers, create_header("Content-Type", "text/html"));
		resp->data = fdata->data;
		status = "HTTP/1.1 200 OK\n";
	} else {
		resp->data = "NOT FOUND";
		char content_length[512];
		sprintf(content_length, "%d", strlen(resp->data));
		llist_append(resp_headers, create_header("Content-Length", content_length));
		llist_append(resp_headers, create_header("Content-Type", "text/plain"));
	}

	resp->status = status;

	return resp;
}
