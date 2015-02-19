#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "csapp.h"
#include "xtiny.h"


void dynamic_adder(int fd, char *prog_args) {
  char *p;
  int n1=0, n2=0;

  char arg1[MAX_LENGTH], arg2[MAX_LENGTH], content[MAXLINE];

  if (prog_args) {
    p = strchr(prog_args, '&');
    *p = '\0';
    strcpy(arg1, prog_args);
    strcpy(arg2, p+1);

    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }

  /* Make the response body */
  
  dprintf(fd, "%s", "HTTP/1.0 200 OK\r\n");
  sprintf(content, "<html> Snapy's Adder service.\r\n<p>");
  sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", 
    content, n1, n2, n1 + n2);
  sprintf(content, "%sThanks for visiting!\r\n</html>", content);

  dprintf(fd, "Content-length: %d\r\n", (int)strlen(content));
  dprintf(fd, "Content-type: text/html\r\n\r\n");
  dprintf(fd, "%s", content); 
  return;

}

void dynamic_factorial(int fd, char* prog_args) {

  char *p;
  int n1=0, n2=0;
  long factorial = 1;
  int i = 0;

  char arg1[MAX_LENGTH], arg2[MAX_LENGTH], content[MAXLINE];
 
  n1 = atoi(prog_args);
  n2 = n1;
  while(n1) {
      factorial *= n1;
      --n1;
  }
  /* Make the response body */
    dprintf(fd, "%s", "HTTP/1.0 200 OK\r\n");
    sprintf(content, "<html> Snapy Factorial Service.\r\n<p>");
    sprintf(content, "%s Factorial of %d is %ld \r\n<p>", 
      content, n2, factorial);
    sprintf(content, "%sThanks for visiting!\r\n</html>", content);

    dprintf(fd, "Content-length: %d\r\n", (int)strlen(content));
    dprintf(fd, "Content-type: text/html\r\n\r\n");
    dprintf(fd, "%s", content); 
    return;
}

void dynamic_default(int fd, char* prog_args) {

  char *p;
  int n1=0, n2=0;

  char arg1[MAX_LENGTH], arg2[MAX_LENGTH], content[MAXLINE];

    if (prog_args) {
    p = strchr(prog_args, '&');
    *p = '\0';
    strcpy(arg1, prog_args);
    strcpy(arg2, p+1);

    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }

  /* Make the response body */
  
  dprintf(fd, "%s", "HTTP/1.0 200 OK\r\n");
  sprintf(content, "<html> Requested service unavailable. \r\n<p>");
  sprintf(content, "%sThanks for visiting!\r\n</html>", content);

  dprintf(fd, "Content-length: %d\r\n", (int)strlen(content));
  dprintf(fd, "Content-type: text/html\r\n\r\n");
  dprintf(fd, "%s", content); 
  return;

}

void dynamic_multiplier(int fd, char *prog_args) {
  char *p;
  int n1=0, n2=0;

  char arg1[MAX_LENGTH], arg2[MAX_LENGTH], content[MAXLINE];

  if (prog_args) {
    p = strchr(prog_args, '&');
    *p = '\0';
    strcpy(arg1, prog_args);
    strcpy(arg2, p+1);

    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }

  /* Make the response body */
  
  dprintf(fd, "%s", "HTTP/1.0 200 OK\r\n");
  sprintf(content, "<html> Snapy's Multipler service.\r\n<p>");
  sprintf(content, "%sThe answer is: %d * %d = %d\r\n<p>", 
    content, n1, n2, n1 * n2);
  sprintf(content, "%sThanks for visiting!\r\n</html>", content);

  dprintf(fd, "Content-length: %d\r\n", (int)strlen(content));
  dprintf(fd, "Content-type: text/html\r\n\r\n");
  dprintf(fd, "%s", content); 
  return;

}