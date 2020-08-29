struct request_info {
	struct sockaddr_storage their_addr;
	int sockfd;
};

struct request_info *create_request_info(struct sockaddr_storage their_addr, int sockfd);
struct llist *get_request_headers(char *request);
