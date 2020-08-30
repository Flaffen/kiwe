struct response {
	char *status;
	struct llist *headers;
	void *data;
	size_t data_len;
	size_t response_len;
};

struct response *get_response(char *method, char *path, struct llist *req_headers);
void free_response(struct response *res);
