#include "tpool.h"
#include "csapp.h"

tpool* tpool_init(int thread_count, int queue_size) {

	int i=0;
	tpool* threadpool = (tpool*) Malloc(sizeof(tpool));
	/* init the pool */
	threadpool->thread_count = 0;
	threadpool->queue_size = queue_size;

	threadpool->head = threadpool->tail = 0;	
	threadpool->threads = (pthread_t*) Malloc(sizeof(pthread_t) * thread_count);
	threadpool->queue = (tpool_args *) Malloc(sizeof(tpool_args) * queue_size);

	if ((pthread_mutex_init(&threadpool->mutex, NULL) != 0) ||
		(pthread_cond_init(&threadpool->cond_var, NULL) != 0)) {
		perror("unable to init mutex \n");
		tpool_destroy(threadpool);
		return NULL;
	}

	for(i=0; i<thread_count; i++) {
		if(pthread_create(&threadpool->threads[i], NULL, snapy_thread_run, (void*)threadpool) != 0) {
			tpool_destroy(threadpool);
		} 
		threadpool->thread_count++;
		printf("Created thread: count %d \n", threadpool->thread_count);
	}
	return threadpool;
}

int tpool_add(tpool* threadpool, tpool_args *args) {

	int next;
	int err=0;
    if(pthread_mutex_lock(&(threadpool->mutex)) != 0) {
        return -1;
    }
    printf("Acquired mutex: adding to pool connfd %d \n", args->connfd);
    next = threadpool->tail + 1;
    next = (next == threadpool->queue_size) ? 0 : next;

    do {
    	/* Are we full ? */
        if(threadpool->count == threadpool->queue_size) {
        	break;
        }

        threadpool->queue[threadpool->tail].connfd = args->connfd;
        printf("Added connfd %d to threadpool \n", args->connfd);
        threadpool->tail = next;
        threadpool->count += 1;

        /* pthread_cond_broadcast */
        if(pthread_cond_signal(&(threadpool->cond_var)) != 0) {
            err = -1;
            break;
        }

    } while(0);

    if(pthread_mutex_unlock(&threadpool->mutex) != 0) {
        err = -1;
    }
    return err;
}


void* snapy_thread_run(void *args) {
	 
	tpool* threadpool = (tpool*) args;
	tpool_args input_args;
	//connect to a socket first.
	int thread_num = -1;
	char sockpath[100];
	int fd, rc;
	struct sockaddr_un client_sock;
	pthread_t self = pthread_self();

	int i;
	for(i=0; i < threadpool->thread_count; i++) {
		if (self == threadpool->threads[i])
			thread_num = i;
	}

	printf("Running thread number %d with tid %d \n", thread_num, (int)self);
	sprintf(sockpath, "/tmp/%d.sock", thread_num);
	printf("socket path %s \n", sockpath);
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("opening stream socket");
        exit(1);
    }

    memset(&client_sock, 0, sizeof(client_sock));
    client_sock.sun_family = AF_UNIX;
    strcpy(client_sock.sun_path, sockpath);

   

	//sleep on a cond_var untill woken up
	for(;;) {

		if (connect(fd, (struct sockaddr *)&client_sock, sizeof(struct sockaddr_un)) < 0) {

        	//close(fd);
        	//perror("connecting stream socket error");
        	//exit(1);
        	//spin wait here till a socket becomes available
        	continue;
    	}
		//Acquire lock to wait on cond_var
		pthread_mutex_lock(&(threadpool->mutex));

		while(threadpool->count == 0) {
			pthread_cond_wait(&threadpool->cond_var, &threadpool->mutex);
		}

		input_args.connfd =  threadpool->queue[threadpool->head].connfd;
		printf("Thread %d got access to queue for connfd %d \n", thread_num, input_args.connfd);
		/* Grab our task */
        
        threadpool->head += 1;
        threadpool->head = (threadpool->head == threadpool->queue_size) ? 0 : threadpool->head;
        threadpool->count -= 1;	

		pthread_mutex_unlock(&threadpool->mutex);
		send_fd(fd, input_args.connfd);
		printf("Sent connfd %d by thread %d \n", input_args.connfd, thread_num);
		Close(input_args.connfd);

	}

	pthread_mutex_unlock(&threadpool->mutex);
	pthread_exit(NULL);
	return (NULL);
}

void tpool_destroy(tpool* threadpool) {
	perror("destroying tpool \n");
	Free(threadpool->threads);
	Free(threadpool->queue);
	Free(threadpool);
}
