#include "csapp.h"
#include "xtiny.h"

void sigchild_handler(int sig) {
    while (waitpid((pid_t)(-1), NULL, WNOHANG) > 0) {}
}

void sigint_handler(int sig) {
    fclose(log_file);
    exit(0);    
}

int main(int argc, char** argv) {

	xtiny_ctx server_ctx;

	int listen_fd, conn_fd, port=0;
	unsigned int clientlen;
	struct sockaddr_in clientaddr;
    char *f = NULL;
	int c;
    struct stat sbuf;
    int thread_count=0, queue_size=0;
    threadpool_t *pool = NULL;

	while((c = getopt(argc, argv, "p:f:t:q:")) != -1 ) {

		switch(c) {
			case 'p':
				port = atoi(optarg);
                break;
			case 'f':
                f = optarg;

                if (stat(f, &sbuf) < 0) {                    
                    printf("Error opening settings file, Exiting \n");
                    exit(-1);
                } 
				strcpy(server_ctx.configfile, f);
                break;
            case 't':
                thread_count = atoi(optarg);
                break;
            case 'q':
                queue_size = atoi(optarg);
                break;
		}
	}

    server_ctx.mode = SNAPY_MODE;
    Signal(SIGCHLD, sigchild_handler);
    Signal(SIGINT, sigint_handler);

    if (thread_count && queue_size) {
        pool = threadpool_create(thread_count, queue_size, 0);
        server_ctx.thread_pool = pool;
    } 
    
	xtiny_init(&server_ctx);
    server_ctx.port = port;

	listen_fd = Open_listenfd(port);

	while(1) {
		clientlen = sizeof(clientaddr);
		conn_fd = Accept(listen_fd, (SA *)&clientaddr, &clientlen);
		conn_serve(conn_fd, &server_ctx);
	}
}

void xtiny_init(xtiny_ctx* ctx) {

	parse_config(ctx);
    strcpy(ctx->server_name, "Snapy");
	log_file = fopen(ctx->log_file, "w+");
    
	/* other init stuff for optimized server */ 

    log_context(ctx);
	log_info("Successfull init for Snapy");
}

void parse_config(xtiny_ctx* server_ctx) {

	FILE *config_file = fopen(server_ctx->configfile, "r");
	char *ptr;

	char buf[MAXLINE];
	if (config_file == NULL) {
		//debug macro
		exit(1);
	} 

	while (fscanf(config_file, "%s\n", buf) != EOF) {
		ptr = strtok(buf, "=");
        if (ptr) {
            if (!strcmp(ptr, "cgi-path")) {
                strcpy(server_ctx->cgi_path, strtok(NULL, "="));
            } else if (!strcmp(buf, "static-root")) {
                strcpy(server_ctx->static_root, strtok(NULL, "="));
            } else if (!strcmp(buf, "log-file")) {
                strcpy(server_ctx->log_file, strtok(NULL, "="));
            } else if (!strcmp(buf, "persistent-port")) {
                server_ctx->dynamic_port = atoi(strtok(NULL, "="));
            } else {
                log_err("Invalid argument in settings file");
            }
        }
        memset(buf, 0, MAXLINE);	
	}
	fclose(config_file);
}

void conn_serve(int connfd, xtiny_ctx *server_ctx) {

	int is_static;
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;

	Rio_readinitb(&rio, connfd);
	Rio_readlineb(&rio, buf, MAXLINE);
	sscanf(buf, "%s %s %s", method, uri, version);

	if(strcasecmp(method, "GET")) {
		clienterror(connfd, method, "501", "Not Implemented", "XTiny does not implement this method");
		return;
	}

	read_requesthrds(&rio);

	/* we read and forget about the headers */
	is_static = parse_uri(uri, filename, cgiargs, server_ctx);

	if (stat(filename, &sbuf) < 0) {                    
		clienterror(connfd, filename, "404", "Not found",
		    "Tiny couldn't find this file");
		return;
    } 

    if (is_static) {
    	if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) { 
	    	clienterror(connfd, filename, "403", "Forbidden",
				"XTiny couldn't read the file");
	    	return;
		}
    	xtiny_serve_static(server_ctx, connfd, filename, sbuf.st_size);
    } else {   
        if (! server_ctx-> mode & SNAPY_MODE) {
            if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { 
                clienterror(connfd, filename, "403", "Forbidden",
                "Tiny couldn't run the CGI program");
                return;
            }
            xtiny_serve_dynamic(server_ctx, connfd, filename, cgiargs);
        } else {
            snapy_serve_dynamic(server_ctx, connfd, filename, cgiargs);
        }
    }
}

void snapy_serve_dynamic(xtiny_ctx *server_ctx, int connfd, char* filename, char* cgiargs) {
    
    /*
        program_name ./snapy/adder
        program_args snap/adder?1&2
    */

    pthread_t tid;
    snapy_thread_args* targs = (snapy_thread_args *) Malloc(sizeof(snapy_thread_args));
    targs->connfd = connfd;
    targs->port = server_ctx->dynamic_port;
    set_snappy_env(server_ctx, cgiargs, targs);
    //Pthread_create(&tid, NULL, snapy_thread_serve_dynamic, targs);
    threadpool_add(server_ctx->thread_pool, snapy_thread_serve_dynamic, targs, 0);
}

void set_snappy_env(xtiny_ctx *server_ctx, char* arg_string, snapy_thread_args* args) {

    char *ptr;
    ptr = index(arg_string, '?');
    if (ptr) {
        strcpy(args->program_args, ptr+1);
        *ptr = '\0';
    }

    ptr = index(arg_string, '/');
    if (ptr) {
        strcpy(args->program_name, ptr+1);
    }
}

void* snapy_thread_serve_dynamic(void* args) {
    
    snapy_thread_args *targs = (snapy_thread_args *)args;  

    /* Open connection to the dynamic content process */    
    struct sockaddr_un client_sock;
    char buf[1024];
    int fd, rc;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("opening stream socket");
        exit(1);
    }

    memset(&client_sock, 0, sizeof(client_sock));
    client_sock.sun_family = AF_UNIX;
    strcpy(client_sock.sun_path, "/tmp/snapy.sock");

    if (connect(fd, (struct sockaddr *)&client_sock, sizeof(struct sockaddr_un)) < 0) {
        close(fd);
        perror("connecting stream socket");
        exit(1);
    }
    log_info("sent connfd %d ", targs->connfd);
    send_fd(fd, targs->connfd);
    //Write(fd, "hello", 5);
    Close(targs->connfd);
    Close(fd);
    Free(args);
}

void *thread_serve_static(void *vargp) {

    int srcfd;
    char *srcp;

    pthread_args *targs = (pthread_args *)vargp;
    Pthread_detach(pthread_self());

    srcfd = Open(targs->filename, O_RDONLY, 0);
    srcp = Mmap(0, targs->filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Write(targs->connfd, srcp, targs->filesize);
    Close(srcfd);
    Munmap(srcp, targs->filesize);
    Close(targs->connfd);

    Free(vargp);

    return NULL;
}

void set_cgi_env(xtiny_ctx *server_ctx, char* arg_string) {

    char *ptr;
    char buf[MAX_STRPORT_LENGTH];
    ptr = index(arg_string,  '?');
    if (ptr) {
        setenv("QUERY_STRING", ptr+1, 1);
        *ptr = '\0';
    } else {
        setenv("QUERY_STRING", "", 1);
    }

    ptr = index(arg_string, '/');
    if (ptr) {
        setenv("SCRIPT_NAME", ptr+1, 1);
    }

    sprintf(buf, "%d", server_ctx->port);
    setenv("SERVER_NAME", server_ctx->server_name, 1);
    setenv("SERVER_PORT", buf, 1);

    /*
    Request Meta-Variables . . . . . . . . . . . . . . . . .  10
            4.1.1.  AUTH_TYPE. . . . . . . . . . . . . . . . . . . .  11
            4.1.2.  CONTENT_LENGTH . . . . . . . . . . . . . . . . .  12
            4.1.3.  CONTENT_TYPE . . . . . . . . . . . . . . . . . .  12
            4.1.4.  GATEWAY_INTERFACE. . . . . . . . . . . . . . . .  13
            4.1.5.  PATH_INFO. . . . . . . . . . . . . . . . . . . .  13
            4.1.6.  PATH_TRANSLATED. . . . . . . . . . . . . . . . .  14
            4.1.7.  QUERY_STRING . . . . . . . . . . . . . . . . . .  15
            4.1.8.  REMOTE_ADDR. . . . . . . . . . . . . . . . . . .  15
            4.1.9.  REMOTE_HOST. . . . . . . . . . . . . . . . . . .  16
            4.1.10. REMOTE_IDENT . . . . . . . . . . . . . . . . . .  16
            4.1.11. REMOTE_USER. . . . . . . . . . . . . . . . . . .  16
            4.1.12. REQUEST_METHOD . . . . . . . . . . . . . . . . .  17
            4.1.13. SCRIPT_NAME. . . . . . . . . . . . . . . . . . .  17
            4.1.14. SERVER_NAME. . . . . . . . . . . . . . . . . . .  17
            4.1.15. SERVER_PORT. . . . . . . . . . . . . . . . . . .  18
            4.1.16. SERVER_PROTOCOL. . . . . . . . . . . . . . . . .  18
            4.1.17. SERVER_SOFTWARE. . . . . . . . . . . . . . . . .  19
    */

}



void xtiny_serve_static(xtiny_ctx *server_ctx, int connfd, char* filename, int filesize) {

    /* write the static content to connfd */

    //int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
    pthread_t tid;
    pthread_args *targs;

    /* Send response headers to client */
    get_filetype(filename, filetype);       
    sprintf(buf, "HTTP/1.0 200 OK\r\n");   
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(connfd, buf, strlen(buf)); 


    /* Send response body to client */


    targs = (pthread_args *) Malloc(sizeof(pthread_args));
    strcpy(targs->filename, filename);
    targs->connfd = connfd;
    targs->filesize = filesize;

    Pthread_create(&tid, NULL, thread_serve_static, targs);
    return;       
}



void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".css"))
        strcpy(filetype, "text/css");
    else if (strstr(filename, ".js"))
        strcpy(filetype, "text/javascript");
    else
        strcpy(filetype, "text/plain");
} 

void xtiny_serve_dynamic(xtiny_ctx *server_ctx, int fd, char* filename, char* cgiargs) {

    char buf[MAXLINE], *emptylist[] = { NULL };
    
    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) {
        set_cgi_env(server_ctx, cgiargs);
        Dup2(fd, STDOUT_FILENO);
        Execve(filename, emptylist, environ); 
        Close(fd);
    }
    //Wait(NULL); //waitpid(-1, &status, 0) we can do better by installing SIGCHLD Handler
}


int parse_uri(char* uri, char* filename, char* cgiargs, xtiny_ctx *server_ctx) {

	char *ptr;

	if (!strstr(uri, server_ctx->cgi_path)) {
		/* static content */
		/* make URI into a complete file name */

		strcpy(filename, server_ctx->static_root);
		strcat(filename, uri);

		if (uri[strlen(uri) - 1] == '/') {
			strcat(filename, "home.html");
		}
		log_info("requested static file name: [%s]", filename);
		log_context(server_ctx);
		return 1;
	} else {

		/*dynamic content*/
		ptr = strstr(uri, server_ctx->cgi_path);
		if (ptr) {
			strcpy(cgiargs, ptr); /*copy the 'cgi-bin/prog?1&2&3' ... */
			//*ptr = '\0';
		} else {
			strcpy(cgiargs, "");
		}

        
        ptr = index(uri,  '?');
        if (ptr) {
            *ptr = '\0';
        }
        
		strcpy(filename, "."); /* file name will be the full path of executable */
		strcat(filename, uri); /* ./cgi-bin/adder | cgi-bin/adder?1&2 */
        
		log_info("cgi-args: [%s]", cgiargs);
		log_context(server_ctx);
		return 0;
	}

}

//TODO replace this with a complete HTTP parser

void read_requesthrds(rio_t *rp) {
	char buf[MAXLINE];

	Rio_readlineb(rp, buf, MAXLINE);
	while(strcmp(buf, "\r\n")) {
		Rio_readlineb(rp, buf, MAXLINE);
	}
	return;
}

void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

void log_context(xtiny_ctx *server_ctx) {
	log_info("dumping xtiny_ctx");
	log_info("config-file: [%s] ", server_ctx->configfile);
	log_info("cgi-path   : [%s] ", server_ctx->cgi_path);
	log_info("static-root: [%s] ", server_ctx->static_root);
	log_info("log file   : [%s] ", server_ctx->log_file);
}

 int send_fd(int socket, int fd_to_send)
 {
  struct msghdr socket_message;
  struct iovec io_vector[1];
  struct cmsghdr *control_message = NULL;
  char message_buffer[1];
  /* storage space needed for an ancillary element with a paylod of length is CMSG_SPACE(sizeof(length)) */
  char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
  int available_ancillary_element_buffer_space;

  /* at least one vector of one byte must be sent */
  message_buffer[0] = 'F';
  io_vector[0].iov_base = message_buffer;
  io_vector[0].iov_len = 1;

  /* initialize socket message */
  memset(&socket_message, 0, sizeof(struct msghdr));
  socket_message.msg_iov = io_vector;
  socket_message.msg_iovlen = 1;

  /* provide space for the ancillary data */
  available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
  memset(ancillary_element_buffer, 0, available_ancillary_element_buffer_space);
  socket_message.msg_control = ancillary_element_buffer;
  socket_message.msg_controllen = available_ancillary_element_buffer_space;

  /* initialize a single ancillary data element for fd passing */
  control_message = CMSG_FIRSTHDR(&socket_message);
  control_message->cmsg_level = SOL_SOCKET;
  control_message->cmsg_type = SCM_RIGHTS;
  control_message->cmsg_len = CMSG_LEN(sizeof(int));
  *((int *) CMSG_DATA(control_message)) = fd_to_send;

  return sendmsg(socket, &socket_message, 0);
 }

