#!/usr/bin/python3

import socket
import time

# Server address and port
server_address = ('localhost', 4000)

# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Connect to the server
    client_socket.connect(server_address)
    print('Connected to server.')
    message = "POST /pages/upload/ HTTP/1.1\r\nHost: localhost:4000\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nteste\r\n"
    client_socket.sendall(message.encode('utf-8'))
    # time.sleep(2)
    message = "5\r\nchunk\r\n"
    client_socket.sendall(message.encode('utf-8'))
    # time.sleep(2)
    message = "0\r\n\r\n"
    client_socket.sendall(message.encode('utf-8'))
    # Receive the response from the server
    response = client_socket.recv(1024).decode('utf-8')
    print('Response from server:\n',response)

except Exception as e:
    print('An error occurred:', str(e))

finally:
    # Close the socket
    client_socket.close()
    print('Socket closed.')