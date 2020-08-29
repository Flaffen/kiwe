#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "llist.h"

void hexdump(void *data, size_t len)
{
	FILE *fp = fopen("hexdump.bin", "w");
	fwrite(data, 1, len, fp);
	fclose(fp);
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

char **create_header(char *key, char *value)
{
	char **headerp = malloc(sizeof(**headerp));
	
	char *keyp = malloc(sizeof(*keyp));
	strcpy(keyp, key);

	char *valuep = malloc(sizeof(*valuep));
	strcpy(valuep, value);

	headerp[0] = keyp;
	headerp[1] = valuep;

	return headerp;
}

void print_header(void *data, void *arg)
{
	char **header = (char **) data;
	printf("%s: %s\n", header[0], header[1]);
}

void print_headers(struct llist *headers_list)
{
	llist_foreach(headers_list, print_header, NULL);
}
