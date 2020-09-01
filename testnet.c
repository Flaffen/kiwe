#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "net.h"

void *handle_http_request(void *data)
{
	int newfd = *((int *) data);

	printf("%d\n", newfd);

	char request[65536];
	while (recv(newfd, request, 65536, 0) > 0) {
		printf("%s", request);
		char *str = "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">";
		char response[512] = {0};
		strcat(response, "HTTP/1.1 200 OK\nContent-Type: text/html\n");
		char content_length[64] = {0};
		sprintf(content_length, "%d", strlen(str));
		strcat(response, "Content-Length: ");
		strcat(response, content_length);
		strcat(response, "\n\n");
		strcat(response, str);

		printf("%s\n", response);

		send(newfd, response, strlen(response), 0);
		memset(request, 0, 65536);
	}

	printf("closed\n");
	close(newfd);

	pthread_exit((void *) 0);
}

int main(void)
{	
	int listenfd = get_listener_socket("3490");
	struct sockaddr_storage their_addr;
	socklen_t sin_size = sizeof(their_addr);
	pthread_t threadid = 0;

	while (1) {
		int newfd = accept(listenfd, (struct sockaddr *) &their_addr, &sin_size);

		pthread_create(&threadid, NULL, handle_http_request, (void *) &newfd);
		pthread_join(threadid, NULL);
		threadid++;
	}

	return 0;
}
