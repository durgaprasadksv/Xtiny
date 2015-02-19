# Xtiny
Xtiny is a concurrent webserver for Internet Services CMU.

Design:
1) Baseline Concurrent Server
 Base line is the tiny server provided for CSAPP.

2) Optimized Baseline is still a Fork/Exec server with concurrency
For each request, the server spawns a thread, which detatches and immediately does a fork() Execv()

3) Xtiny is an improvement to Optimized baseline removing the need to Fork/Exec for every 
request by passing the dynamic request args and connection descriptor to a helper process 
over a UNIX domain socket. The helper process server dynamic content by reading the request args 
and calling the appropriate function. All dynamic content generating functions must accept an input
file descriptor and write their reponse on the passed fd. 

In my Experiments I have found that Baseline peak response rate is about 1000 req/sec
and Optimized is about 4000 req/sec and Xtiny can serve upto 11500 req/sec. 
Note that network performance is very essential to hit these numbers.

The results are more obvious when you run both the servers on a memory constrained machine. 
The httperf results for Optimized and Xtiny are in the optimized.results and improved.results respectively
