#include "csapp.h"
#include "xtiny.h"

int queue_len = 2000;
int thread_count = 20;

typedef struct request_args {
  char program_name[MAX_LENGTH];
  char program_args[MAX_LENGTH];
  int targetfd; //to write back to client
} request_args;

typedef struct worker_args {
  int targetfd;
  int connfd;
  char program_args[MAX_LENGTH];
  char program_name[MAX_LENGTH];
} worker_args;

void* dynamic_worker_thread(void *args);
void dynamic_adder(int fd, char *prog_args);

int adder(int fd) {

    char content[MAXLINE];
    int n1=1, n2=2;
    
    /* Make the response body */
 	
    dprintf(fd, "%s", "HTTP/1.0 200 OK\r\n");
    sprintf(content, "<html> THE Internet addition portal.\r\n<p>");
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", 
	    content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting!\r\n</html>", content);

    dprintf(fd, "Content-length: %d\r\n", (int)strlen(content));
    dprintf(fd, "Content-type: text/html\r\n\r\n");
	 
    dprintf(fd, "%s", content);	
    return 1;
}

int main(int argc, char *argv[]) {

	struct sockaddr_un addr;
  int fd,cl,rcvdfd;
  request_args req_args;
  int request_size = 0;
  const char* socket_path;
  int head = 0;

  if (argc < 2) {
     printf("usage: snapy <DOMAIN_SOCKET PATH> \n");
     exit(0);
  }

  socket_path= argv[1];

  if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(-1);
  }
  
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

  unlink(socket_path);

  if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind error");
    exit(-1);
  }

  if (listen(fd, 5) == -1) {
    perror("listen error");
    exit(-1);
  }

  threadpool_t *pool;
  pool = threadpool_create(thread_count, queue_len, 0);
  worker_args *worker_args_queue = (worker_args*) Malloc(sizeof(queue_len));


  while (1) {

	    if ((cl = accept(fd, NULL, NULL)) == -1) {
	      perror("accept error");
	      continue;
	    }

	    rcvdfd = recv_fd(cl);
	    request_size = Read(cl, &req_args, sizeof(request_args));
      worker_args *args = (worker_args*) Malloc(sizeof(worker_args));

      /*
      &worker_args_queue[head];
      head = (head + 1) % queue_len;
      */
      
      args->targetfd = rcvdfd;
      args->connfd = cl;
      strcpy(args->program_args, req_args.program_args);
      strcpy(args->program_name, req_args.program_name);
      threadpool_add(pool, dynamic_worker_thread, args, 0);
	}
  
  Free(worker_args_queue);
  return 0;
}


void* dynamic_worker_thread(void *args) {

    worker_args *rargs = (worker_args*)args;
    int fd = rargs->targetfd;

    /* we can call any program here depending upon
    the args->program_name */
    
    if (!strcmp(rargs->program_name, "adder")) {
        dynamic_adder(fd, rargs->program_args);
    } else if (!strcmp(rargs->program_name, "multiplier"))
        dynamic_multiplier(fd, rargs->program_args);
    else if (!strcmp(rargs->program_name, "factorial"))
        dynamic_factorial(fd, rargs->program_args);
    else 
        dynamic_default(fd, rargs->program_args);
    
    //dynamic_adder(fd, rargs->program_args);
    Close(fd);
    Close(rargs->connfd);
    Free(args);
    return (NULL);
}
  

int recv_fd(int socket)
 {
  int sent_fd, available_ancillary_element_buffer_space;
  struct msghdr socket_message;
  struct iovec io_vector[1];
  struct cmsghdr *control_message = NULL;
  char message_buffer[1];
  char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];

  /* start clean */
  memset(&socket_message, 0, sizeof(struct msghdr));
  memset(ancillary_element_buffer, 0, CMSG_SPACE(sizeof(int)));

  /* setup a place to fill in message contents */
  io_vector[0].iov_base = message_buffer;
  io_vector[0].iov_len = 1;
  socket_message.msg_iov = io_vector;
  socket_message.msg_iovlen = 1;

  /* provide space for the ancillary data */
  socket_message.msg_control = ancillary_element_buffer;
  socket_message.msg_controllen = CMSG_SPACE(sizeof(int));

  if(recvmsg(socket, &socket_message, O_CLOEXEC) < 0)
   return -1;

  if(message_buffer[0] != 'F')
  {
   /* this did not originate from the above function */
   return -1;
  }

  if((socket_message.msg_flags & MSG_CTRUNC) == MSG_CTRUNC)
  {
   /* we did not provide enough space for the ancillary element array */
   return -1;
  }

  /* iterate ancillary elements */
   for(control_message = CMSG_FIRSTHDR(&socket_message);
       control_message != NULL;
       control_message = CMSG_NXTHDR(&socket_message, control_message))
  {
   if( (control_message->cmsg_level == SOL_SOCKET) &&
       (control_message->cmsg_type == SCM_RIGHTS) )
   {
    sent_fd = *((int *) CMSG_DATA(control_message));
    return sent_fd;
   }
  }

  return -1;
 }



ssize_t
read_fd(int fd, void *ptr, size_t nbytes, int *recvfd)
{
    struct msghdr   msg;
    struct iovec    iov[1];
    ssize_t         n;

    union {
      struct cmsghdr    cm;
      char              control[CMSG_SPACE(sizeof(int))];
    } control_un;
    struct cmsghdr  *cmptr;

    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    iov[0].iov_base = ptr;
    iov[0].iov_len = nbytes;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    if ( (n = recvmsg(fd, &msg, 0)) <= 0)
        return(n);

    if ( (cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
        cmptr->cmsg_len == CMSG_LEN(sizeof(int))) {
        if (cmptr->cmsg_level != SOL_SOCKET)
            perror("control level != SOL_SOCKET");
        if (cmptr->cmsg_type != SCM_RIGHTS)
            perror("control type != SCM_RIGHTS");
        *recvfd = *((int *) CMSG_DATA(cmptr));
    } else
        *recvfd = -1;       /* descriptor was not passed */


    return(n);
}
/* end read_fd */

ssize_t
Read_fd(int fd, void *ptr, size_t nbytes, int *recvfd)
{
    ssize_t     n;

    if ( (n = read_fd(fd, ptr, nbytes, recvfd)) < 0)
        perror("read_fd error");

    return(n);
}
