struct request_info {
	struct sockaddr_storage their_addr;
	int sockfd;
	struct cache *cache;
};

struct request_info *create_request_info(struct sockaddr_storage their_addr, int sockfd, struct cache *cache);
struct llist *get_request_headers(char *request);
