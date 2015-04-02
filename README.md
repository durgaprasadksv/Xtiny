# xtiny

XTiny is a high performance concurrent server 
written for Internet Services course at CMU. 
http://www.ece.cmu.edu/~ece845/

Design:

A request is accepted and its execution is deffered 
to a thread pool. A thread picks up the request from 
the thread pool queue, serves the request and closes
the connection. Note that only HTTP 1.0 is supported 
(No persistent connection support). 

Static requests are served my mmap'ing the static file. 

Dynamic Requests:
CGI - As the CGI protocol mandates, each cgi request
is handled by forking a process and dup'ing the 
standard out.

Snap: In the snap mode dynamic requests are handled 
by a separate process. A thread from the threadpool
offloads the dynamic serving to another process using
UNIX Domain Sockets by passing the connection descriptor
and the HTTP arguments to the dynamic serving process. 
The dynamic request serving process can form the response 
and write it to the passed connection descriptor and close
the request. 

Starting:
./xtiny -p 8000 -f settings.conf -q 50 -t 50 -d /tmp/domain.sock
(q is the queue size and t is the number of threads in the thread pool)

Start the dynamic serving process using 
./content /tmp/domain.sock
