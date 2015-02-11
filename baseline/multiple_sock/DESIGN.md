The optmized protocol has the following design:

1) The baseline CGI is slower because of one Fork() per dynamic request
2) No parallel static file serving. 


To mitigate (1):
Typically dynamic content is always served through other processes like python/perl executing webapplication code and returning the http response. There is no way to avoid a fork() call unless the process that is supposed to serve dynamic content is already running:

Design:
Dynamic processes are persistent. When the webserver gets a dynamic content request, we offload it on to processes running on predefined ports. All the arguments supplied by the user are passing to the process by writing to the socket including the connected descriptor. The dynamic process then reads the request, executes and writes the dynamic content on to the descriptor we have passed. 


request -> conndf -> threadpool_queue {connfd + arguments} -> thread {SCRIPT_NAME, CONNFD, ARGS} -> p1 

processes 
p1
p2
p3
p4
p5

