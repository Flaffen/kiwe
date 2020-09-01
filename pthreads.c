#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void *threadfunc(void *data)
{
	printf("hello from thread!\n");

	pthread_exit((void *) 0);
}

int main(void)
{
	pthread_t threadid = 0;

	pthread_create(&threadid, NULL, threadfunc, NULL);
	pthread_detach(threadid);
	threadid++;

	return 0;
}
