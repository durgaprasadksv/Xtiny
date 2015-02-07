#ifndef _XTINY_H_
#define _XTINY_H_

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "unp.h"

#define MAX_LENGTH 1024
#define MAX_STRPORT_LENGTH 20

typedef struct xtiny_ctx {
	char configfile[MAX_LENGTH];
	char cgi_path[MAX_LENGTH];
	char static_root[MAX_LENGTH];
	char log_file[MAX_LENGTH];
	char server_name[MAX_LENGTH];
	int port;
	/*other parameters you deem fit */
} xtiny_ctx;

typedef struct pthread_args {
	char filename[MAX_LENGTH];
	int connfd;
	int filesize;
} pthread_args;

void *thread_serve_static(void *vargp);
void get_filetype(char *filename, char *filetype);
void conn_serve(int fd, xtiny_ctx *ctx);
int parse_uri(char* uri, char* filename, char* cgiargs, xtiny_ctx *server_ctx);
void xtiny_init(xtiny_ctx* ctx);
void parse_config(xtiny_ctx* ctx);
void read_requesthrds(rio_t *ri);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void log_context(xtiny_ctx *ctx);
void xtiny_serve_static(xtiny_ctx *server_ctx, int fd, char* filename, int filesize);
void xtiny_serve_dynamic(xtiny_ctx *server_ctx, int fd, char* filename, char* filesize);
void set_cgi_env(xtiny_ctx *server_ctx, char* arg_string);
FILE *log_file;
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_err(M, ...) fprintf(log_file, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define log_warn(M, ...) fprintf(log_file, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define log_info(M, ...) fprintf(log_file, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }
#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }
#define check_mem(A) check((A), "Out of memory.")
#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }


#endif // _XTINY_H_