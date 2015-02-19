import socket
import sys
import os

server_address = '/tmp/snappy.sock'

# Make sure the socket does not already exist
try:
    os.unlink(server_address)
except OSError:
    if os.path.exists(server_address):
        raise
# Create a UDS socket
sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
print >>sys.stderr, 'starting up on %s' % server_address
sock.bind(server_address)

sock.listen(1)
while True:
    # Wait for a connection
    print >>sys.stderr, 'waiting for a connection'
    connection, client_address = sock.accept()
    print >>sys.stderr, 'connection from', client_address

    # Receive the data in small chunks and retransmit it
    data = connection.recv(1024)
    domargs = data.split('\t')
    connfd = int(domargs[0].strip().split(':')[1])
    program_name = domargs[1].strip().split(':')[1]
    program_args = domargs[2].strip().split(':')[1]
    print connfd
    os.write(connfd, "hello there")