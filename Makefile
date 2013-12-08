tcp: src/tcp_client.c src/tcp_server.c
	gcc -o client.o src/tcp_client.c
	gcc -o server.o src/tcp_server.c

udp: src/udp_client.c src/udp_server.c src/udp_basic.c
	gcc -o bin/udp_basic.o -c src/udp_basic.c
	gcc -o bin/utils.o -c src/utils.c
	gcc -o bin/udp_server.o -c src/udp_server.c
	gcc -o bin/udp_client.o -c src/udp_client.c
	gcc -o server.o bin/udp_server.o bin/udp_basic.o
	gcc -o client.o bin/udp_client.o bin/udp_basic.o

saw: src/udp_client.c src/udp_server.c src/saw.c
	gcc -o bin/saw.o -c src/saw.c
	gcc -o bin/utils.o -c src/utils.c
	gcc -o bin/udp_server.o -c src/udp_server.c
	gcc -o bin/udp_client.o -c src/udp_client.c
	gcc -o server.o bin/utils.o bin/udp_server.o bin/saw.o -g
	gcc -o client.o bin/utils.o bin/udp_client.o bin/saw.o -g
