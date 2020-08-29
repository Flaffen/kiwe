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

struct request_info *create_request_info(struct sockaddr_storage their_addr, int sockfd) {
	struct request_info *re = malloc(sizeof(*re));

	re->their_addr = their_addr;
	re->sockfd = sockfd;

	return re;
}

struct llist *get_request_headers(char *request)
{
	char line[512] = {0};
	struct llist *list = llist_create(); 

	while (consume_http_line(&request, line) != -1) {
		char key[64] = {0};
		char value[512] = {0};
		sscanf(line, "%s %s", key, value);
		key[strlen(key) - 1] = '\0';

		char *shortkey = malloc(strlen(key) + 1);
		strcpy(shortkey, key);

		char *shortvalue = malloc(strlen(value) + 1);
		strcpy(shortvalue, value);

		char **entry = malloc(sizeof(*entry));
		entry[0] = shortkey;
		entry[1] = shortvalue;

		llist_append(list, entry);

		memset(line, 0, 512);
	}

	return list;
}
