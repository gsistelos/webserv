#!/usr/bin/python3

import socket
import time

# Server address and port
server_address = ('localhost', 4081)

# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Connect to the server
    client_socket.connect(server_address)
    print('Connected to server.')

    message = "POST /cgi-bin/upload2.py HTTP/1.1\r\n"
    message += "Transfer-Encoding: chunked\r\n\r\n"
    client_socket.sendall(message.encode('utf-8'))

    input("Enter to continue...")
    message = "5\r\nhelloaa"
    client_socket.sendall(message.encode('utf-8'))


    input("Enter to continue...")
    message = "3\r\nbye"
    client_socket.sendall(message.encode('utf-8'))
    
    
    input("Enter to continue...")
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
