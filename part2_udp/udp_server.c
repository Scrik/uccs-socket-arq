/* Echo server using UDP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#include "udp_stop-and-wait.h"

#define SERVER_UDP_PORT    5000
#define MAXLEN             1024

void print_usage(char *pname)
{
  fprintf(stderr,
  "Usage: %s [-p port] drop_rate protocol\n\
   drop_rate: [0-100], the percent chance of dropping a packet\n\
   protocol: 0=Basic, 1=Stop-and-Wait, 2=Go Back N, 3=Selective Repeat\n", pname);
}

int main(int argc, char **argv)
{
   int    sd, client_len, n, port = SERVER_UDP_PORT, dropRate, protocol, num_frames, data_size;
   char    buf[MAXLEN], *pname;
   struct    sockaddr_in    server, client;

   // Seed the random number generator
   srand(time(NULL));

    pname = argv[0];
    argc--;

    if (argc > 0) {
      printf("argc = %d\n", argc);
       if(strcmp(*++argv, "-p") == 0) {
          port = atoi(*++argv);
          argc -= 2;
       } else {
         --argv;
       }
      printf("argc = %d\n", argc);
       if(argc == 2) {
        dropRate = atoi(*++argv);
        protocol = atoi(*++argv);
       } else {
        print_usage(pname);
        exit(1);
       }
    } else {
       print_usage(pname);
       exit(1);
    }


   printf("Using port=%d, drop_rate=%d, protocol=%d\n", port, dropRate, protocol);
      
   /* Create a datagram socket */
   if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
      fprintf(stderr, "END [FAILURE] Can't create a socket\n");
      exit(1);
   }


   /* Bind an address to the socket */
   bzero((char *)&server, sizeof(server));
   server.sin_family = AF_INET;
   server.sin_port = htons(port);
   server.sin_addr.s_addr = htonl(INADDR_ANY);
   if (bind(sd, (struct sockaddr *)&server, 
            sizeof(server)) == -1) {
      fprintf(stderr, "END [FAILURE] Can't bind name to socket\n");
      exit(1);
   }

   while (1) {
      client_len = sizeof(client);

      // printf("START Receive UDP\n");
      // if ((n = recvfrom(sd, buf, MAXLEN, 0, 
      // (struct sockaddr *)&client, &client_len)) < 0) {
      //       fprintf(stderr, "END [FAILURE] Can't receive datagram\n");
      //       continue;
      // }
      // printf("END [SUCCESS] Receive UDP\n");

      if( (num_frames = receive_udp(client_len, client, sd, buf, &data_size)) == -1 ) {
         continue;
      }

      printf("~~~~ data_size from receive: %d\n", data_size);

      send_udp(client_len, client, sd, buf, num_frames, data_size, dropRate);
   }
   close(sd);
   return(0);
}
