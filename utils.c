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

char **create_header_int(char *key, int value)
{
	char valuestr[512];
	sprintf(valuestr, "%d", value);

	return create_header(key, valuestr);
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

void get_header_string(char **header, char str[])
{
	char *key = header[0];
	char *value = header[1];

	sprintf(str, "%s: %s", key, value);
}

int content_length_comparison_function(void *arg1, void *arg2)
{
	char *str = (char *) arg1;
	char **header = (char **) arg2;

	return strcmp(str, header[0]);
}

int get_content_length(struct llist *headers)
{
	if (headers == NULL)
		return -1;

	void *foundv = llist_find(headers, "Content-Length", content_length_comparison_function);

	if (foundv != NULL) {
		char **found = (char **) foundv;
		return atoi(found[1]);
	}

	return -1;
}

void get_header_len(void *data, void *arg)
{
	char **header = (char **) data;
	size_t *totalsizep = (size_t *) arg;

	// Key and value sizes
	*totalsizep += strlen(header[0]) + strlen(header[1]);

	// Colon
	*totalsizep += 1;

	// Space between key and value
	*totalsizep += 1;

	// Newline
	*totalsizep += 1;

	*totalsizep += 2;
}

size_t get_headers_len(struct llist *headers)
{
	size_t totalsize = 0;
	llist_foreach(headers, get_header_len, &totalsize);


	return totalsize;
}
