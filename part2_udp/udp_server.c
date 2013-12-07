/* Echo server using UDP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#include "udp_stop-and-wait.h"

#define SERVER_UDP_PORT    5000
#define MAXLEN             4096
#define DROP_RATE          5     // Out of 100, % chance of dropping

int main(int argc, char **argv)
{
   int    sd, client_len, port, n;
   char    buf[MAXLEN];
   struct    sockaddr_in    server, client;

   // Seed the random number generator
   srand(time(NULL));

   switch(argc) {
   case 1:
      port = SERVER_UDP_PORT;
      break;
   case 2:
      port = atoi(argv[1]);
      break;
   default:
      fprintf(stderr, "Usage: %s [port]\n", argv[0]);
      exit(1);
   }

   printf("Using port %d\n", port);
      
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
      if ((n = recvfrom(sd, buf, MAXLEN, 0, 
      (struct sockaddr *)&client, &client_len)) < 0) {
            fprintf(stderr, "END [FAILURE] Can't receive datagram\n");
            continue;
      }

      sendStopAndWait(client_len, client, sd, buf, n);
   }
   close(sd);
   return(0);
}

/**
 * The initial implementation, with no reliability.
 * It randomly drops packets based on constant at top of file
 **/
int sendBasic(int client_len, struct sockaddr_in client, int sd, char *buf, int n)
{

   printf("START [BASIC] Sending datagram...\n");
   printf("   CLIENT => client_len=%d, n=%d\n", client_len, n);

   // Modify the buffer to simulate dropped packets
   int m = 0;  // Size of modified buffer
   int i, j, r;   // i: index of original buffer
               // j: index of new buffer
               // r: random number deciding whether to drop packet
   char droppedBuf[n];
   printf("   START iterating over packets! BufIn.len = %d\n", n);
   printf("      ");
   for(i=0, j=0; i < n; i++) {
      r = 1 + (rand() % 100); // range of 1-100
      printf("(%2d)", i);
      if( r > DROP_RATE ) {
      // if( i%2 == 0 ) {
         printf(".");
         // Don't drop the packet
         droppedBuf[j] = buf[i];
         j++;
      } else {
         printf("DROP\n      ");
         // By not copying over the element, we "drop" it
         // DO NOTHING, intentionally
      }
   }
   printf("\n   END iterating over packets! BufOut.len = %d\n", j);

   if (sendto(sd, droppedBuf, j, 0, 
   (struct sockaddr *)&client, client_len) != n) {
         fprintf(stderr, "END [FAILURE] Can't send datagram\n");
         return 1;
   } else {
      printf("END [SUCCESS] Datagram sent\n");
      return 0;
   }
}