struct response {
	char *status;
	struct llist *headers;
	void *data;
};

struct response *get_response(char *method, char *path, struct llist *req_headers);
void free_response(struct response *res);
