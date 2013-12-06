tcp: part1_tcp/tcp_client.c part1_tcp/tcp_server.c
	gcc -o client.o part1_tcp/tcp_client.c
	gcc -o server.o part1_tcp/tcp_server.c

udp: part2_udp/udp_client.c part2_udp/udp_server.c
	gcc -o client.o part2_udp/udp_client.c
	gcc -o server.o part2_udp/udp_server.c
