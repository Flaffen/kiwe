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
	struct response *resp = malloc(sizeof(*resp));
	struct llist *resp_headers = llist_create();
	char *status;
	char *filename = path + 1;

	status = "HTTP/1.1 404 NOT FOUND";
	resp->headers = resp_headers;

	if (strcmp(path, "/") == 0) {
		struct file_data *fdata = load_file("index.html");

		status = "HTTP/1.1 200 OK\n";
		llist_append(resp->headers, create_header_int("Content-Length", fdata->len));
		llist_append(resp->headers, create_header("Content-Type", "text/html"));
		resp->data = fdata->data;
	} else if (strcmp(path, "/") != 0) {
		char full_filename[128] = {0};
		strcat(full_filename, "serverroot/");
		strcat(full_filename, filename);
		struct file_data *fdata = load_file(full_filename);

		if (fdata == NULL) {
			return get_404(resp);
		}

		char *mime_type = mime_type_get(filename);
		status = "HTTP/1.1 200 OK\n";
		llist_append(resp->headers, create_header_int("Content-Length", fdata->len));
		llist_append(resp->headers, create_header("Content-Type", mime_type));
		resp->data = fdata->data;
	} else {
		get_404(resp);
	}

	resp->status = malloc(sizeof(*status));
	strcpy(resp->status, status);

	return resp;
}

void free_response(struct response *res)
{
	free(res->status);
	llist_destroy(res->headers);
	free(res->data);
	free(res);
}
