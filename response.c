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
#include "mime.h"

struct response *get_404(struct response *resp)
{
	struct file_data *fdata = load_file("404.html");

	llist_append(resp->headers, create_header_int("Content-Length", fdata->len));
	llist_append(resp->headers, create_header("Content-Type", "text/html"));
	char *status = "HTTP/1.1 404 NOT FOUND\n";
	resp->status = malloc(sizeof(resp->status));
	strcpy(resp->status, status);
	resp->data = fdata->data;
	
	return resp;
}

struct response *get_response(char *method, char *path, struct llist *req_headers)
{
	struct response *res = malloc(sizeof(struct response));

	char *status;
	struct llist *headers = llist_create();
	char *data;
	size_t data_len;
	size_t response_len;

	char *filename = path + 1;

	char *qm = strchr(filename, '?');
	if (qm != NULL) {
		*qm = '\0';
	}

	if (strcmp(path, "/") == 0) {
		filename = "index.html";
	}

	char fullpath[512] = {0};
	strcat(fullpath, "serverroot/");
	strcat(fullpath, filename);
	struct file_data *fdata = load_file(fullpath);

	if (fdata != NULL) {
		status = "HTTP/1.1 200 OK\n";
		char content_length[64];
		sprintf(content_length, "%zu", fdata->len);
		data = fdata->data;
		data_len = fdata->len;
		char *mime_type = mime_type_get(fullpath);
		printf("%s\n", mime_type);
		llist_append(headers, create_header("Content-Type", mime_type));
		llist_append(headers, create_header("Content-Length", content_length));
	} else {
		status = "HTTP/1.1 404 NOT FOUND\n";
		struct file_data *fdata = load_file("404.html");
		char content_length[64];
		sprintf(content_length, "%zu", fdata->len);
		data = fdata->data;
		data_len = fdata->len;
		llist_append(headers, create_header("Content-Type", "text/html"));
		llist_append(headers, create_header("Content-Length", content_length));
	}

	res->status = malloc(sizeof(strlen(status) + 1));
	strcpy(res->status, status);

	res->headers = headers;

	res->data_len = data_len;
	res->data = malloc(data_len);
	memcpy(res->data, data, data_len);

	return res;
}

void free_response(struct response *res)
{
	// nope.
}
