CS5220 Project 2: Socket-based ARQ Protocol Programming
#######################################################

## 3.1. A file server in TCP

The source code of both client side and server side are downloadable; TCP Client in C and TCP Server in C for reference. Current client code displays the (text) file on the standard output. Modify client side so that a binrary file, transferred from the server, will be saved as a file in the local directory. Make both sides running on Unix computers (by remote access). Note that currently three servers are accessible, Blanca, Shavano and Windom. Since those Unix machines are sharing the same file system, you may create a new directory to save the received file at the client side.
```
Server side: TCP_server
Client side: TCP_client server_hostname filename 
```

TCP_client is the executable program. server_hostname is the hostname of the machine you use as the server (use windom). filename is the file that client wants to read from the server. We assume that the server has the requested file (you can store several PDF files there). Use the PDF file TEST FILE to test your programs. You may use binary I/O in the file operations. You may also want to save the transmitted file in a different directory due to the use of a shared file system in our Unix computers (by remote access).

You should be able to open the test PDF file on the client machine after the TCP transmission is completed. Note that you may use 2XYZ as the port number in your program, in which XYZ are the last 3 digits of your student ID. Doing so would avoid that multiple programs/processes use the same port for communication.

## 3.2. A file server in UDP (with flow control)

Modify the source code of 3.1 using UDP sockets (instead of TCP sockets). Make both sides running on Unix computers (by remote access). See UDP Client in C and UDP Server in C for reference. Then, enhance the reliability of your code of 3.2(a) by implementing two ARQ algorithms; stop-and-wait (SW), and one of go back N (GBN) and selective repeat (SR). Refer to Chapter 5 of the book for sequencing, ACKs, and retransmission mechanisms. The communication should be reliable, assuring that the client got whole file correctly. Since UDP indeed is indeed quite reliable for communications within a LAN, use a random generator/function to "purposely" drop X% of data blocks BEFORE they are delivered from the server side. Each UDP segment should be less than 4KB, so that you can have enough segments to experience a few segment losses.

Server side: UDP_server loss_probability protocol_type Client side: UDP_client server_hostname file_name protocol_type Parameter loss_probability is the dropping probability (an integer) as a command-line input of the server code. Parameter protocol_type is the ARQ type implemented (1 for SW, 2 for GBN, 3 for SR). server_hostname is the hostname of the machine you use as the server (e.g., use WINDOM server). filename is the file that client wants to read from the server. Clearly, we assume that the server has the requested file. Use the PDF file TEST FILE to test your programs. You should be able to open the test PDF file on the client machine. You may want to save the transmitted file in a different directory due to the use of a shared file system in our Unix/Unix computers. Again, you may use 2XYZ as the port number in your program, in which XYZ are the last 3 digits of your student ID. Doing so would avoid that multiple programs/processes use the same port for communication.

## What to turn in:

Each student will do the project(s) individually.
For this project you will email me at zbo@cs.uccs.edu one ZIP file (in the name of P2_Yourlastnames) of all source code, together with a technical report specifying 1) how to compile and execute your program; 2) the self-testing results of your program and any important feature you want to specify (if your program does not work, please notify it clearly). Your program should print the trace of flow control for the lost frames (i.e., Seq#, ACK#, Retransmission#) into an output text file. Please also have the project report in a hardcopy at the project submission time. Make sure your code can work in Unix machines (by remote access). We may check your project during the class time by remotely accessing servers.

You may search the Internet for more helpful information on socket-based network programming.
