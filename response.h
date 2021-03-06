struct response {
	char *status;
	struct llist *headers;
	void *data;
	size_t data_len;
	size_t response_len;
};

struct response *get_response(struct response *resp, char *method, char *path, struct llist *req_headers, struct cache *cache);
void free_response(struct response *res);
void append_header_to_response(void *data, void *arg);
void assemble_response_data(struct response *resp, char response_data[]);
