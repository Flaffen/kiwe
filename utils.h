#include "llist.h"

void hexdump(void *data, size_t len);
int consume_http_line(char **request, char *line);
char **create_header(char *key, char *value);
void print_header(void *data, void *arg);
void print_headers(struct llist *headers_list);
void get_header_string(char **header, char str[]);
