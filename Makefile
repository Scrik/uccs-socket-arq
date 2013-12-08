tcp: part1_tcp/tcp_client.c part1_tcp/tcp_server.c
	gcc -o client.o part1_tcp/tcp_client.c
	gcc -o server.o part1_tcp/tcp_server.c

udp: part2_udp/udp_client.c part2_udp/udp_server.c part2_udp/udp_basic.c
	gcc -o part2_udp/bin/udp_basic.o -c part2_udp/udp_basic.c
	gcc -o part2_udp/bin/udp_server.o -c part2_udp/udp_server.c
	gcc -o part2_udp/bin/udp_client.o -c part2_udp/udp_client.c
	gcc -o server.o part2_udp/bin/udp_server.o part2_udp/bin/udp_basic.o
	gcc -o client.o part2_udp/bin/udp_client.o part2_udp/bin/udp_basic.o

saw: part2_udp/udp_client.c part2_udp/udp_server.c part2_udp/udp_stop-and-wait.c
	gcc -o part2_udp/bin/udp_stop-and-wait.o -c part2_udp/udp_stop-and-wait.c
	gcc -o part2_udp/bin/udp_server.o -c part2_udp/udp_server.c
	gcc -o part2_udp/bin/udp_client.o -c part2_udp/udp_client.c
	gcc -o server.o part2_udp/bin/udp_server.o part2_udp/bin/udp_stop-and-wait.o -g
	gcc -o client.o part2_udp/bin/udp_client.o part2_udp/bin/udp_stop-and-wait.o -g
