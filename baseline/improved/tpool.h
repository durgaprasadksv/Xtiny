
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_ARG_SIZE 1024


typedef struct tpool_args {
	int connfd;
	char args[MAX_ARG_SIZE];
} tpool_args;

typedef struct tpool {
	pthread_mutex_t mutex;
	pthread_cond_t cond_var;
	pthread_t* threads;
	tpool_args *queue;
	int queue_size;
	int thread_count;
	int head;
	int tail;
	int count;
	
} tpool;

tpool* tpool_init(int thread_count, int queue_size);
int tpool_add(tpool* threadpool, tpool_args *args);
void* snapy_thread_run(void *args);
void tpool_destroy(tpool* threadpool);
extern int send_fd(int socket, int fd_to_send);