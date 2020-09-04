#include "llist.h"

void hexdump(void *data, size_t len);
int consume_http_line(char **request, char *line);
char **create_header(char *key, char *value);
char **create_header_int(char *key, int value);
void print_header(void *data, void *arg);
void print_headers(struct llist *headers_list);
void get_header_string(char **header, char str[]);
int get_content_length(struct llist *headers);
size_t get_headers_len(struct llist *headers);
