/* Echo server using UDP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#include "../inc/utils.h"
#include "../inc/saw.h"


void print_usage(char *pname)
{
  fprintf(stderr,
  "Usage: %s [-p port] drop_rate protocol\n\
   drop_rate: [0-100], the percent chance of dropping a packet\n\
   protocol: 0=Basic, 1=Stop-and-Wait, 2=Go Back N, 3=Selective Repeat\n", pname);
}

int main(int argc, char **argv)
{
   int    sd, client_len, n, port = SERVER_UDP_PORT, dropRate, protocol, num_frames, data_size, bytes;
   char    buf[BUF_SIZE], *pname;
   struct    sockaddr_in    server, client;

   // Seed the random number generator
   srand(time(NULL));

    pname = argv[0];
    argc--;

    if (argc > 0) {
       if(strcmp(*++argv, "-p") == 0) {
          port = atoi(*++argv);
          argc -= 2;
       } else {
         --argv;
       }
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
      printf("END [FAILURE] Can't create a socket\n");
      exit(1);
   }


   /* Bind an address to the socket */
   bzero((char *)&server, sizeof(server));
   server.sin_family = AF_INET;
   server.sin_port = htons(port);
   server.sin_addr.s_addr = htonl(INADDR_ANY);
   if (bind(sd, (struct sockaddr *)&server, 
            sizeof(server)) == -1) {
      printf("END [FAILURE] Can't bind name to socket\n");
      exit(1);
   }

   while (1) {
      client_len = sizeof(client);

      switch(protocol) {
        case STOP_AND_WAIT:
          if( (bytes = receive_saw(&client_len, &client, sd, buf, &data_size, dropRate)) == -1 ) {
             continue;
          }
          num_frames = calculateNumFrames(bytes, data_size);

          printf("ECHO %d x %d B Frames of total size %d B\n", num_frames, data_size, bytes);

          send_saw(client_len, client, sd, buf, num_frames, data_size, bytes, dropRate);

          break;
        case GO_BACK_N:
          printf("END [FAILURE] GO BACK N not implemented yet\n");
          return(1);
          break;
        case SELECTIVE_REPEAT:
          printf("END [FAILURE] SELECTIVE REPEAT not implemented\n");
          return(1);
          break;
        default:
          print_usage(pname);
          return(1);
          break;
      }
   }
   close(sd);
   return(0);
}
