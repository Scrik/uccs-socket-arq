/* Echo server using UDP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define SERVER_UDP_PORT     5000
#define MAXLEN      		4096
#define DROP_RATE			50

int main(int argc, char **argv)
{
	printf("FOOO!");
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

   printf("Using port %d", port);
      
   /* Create a datagram socket */
   if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
      fprintf(stderr, "Can't create a socket\n");
      exit(1);
   }


   /* Bind an address to the socket */
   bzero((char *)&server, sizeof(server));
   server.sin_family = AF_INET;
   server.sin_port = htons(port);
   server.sin_addr.s_addr = htonl(INADDR_ANY);
   if (bind(sd, (struct sockaddr *)&server, 
   sizeof(server)) == -1) {
      fprintf(stderr, "Can't bind name to socket\n");
      exit(1);
   }

   while (1) {
      client_len = sizeof(client);
      if ((n = recvfrom(sd, buf, MAXLEN, 0, 
      (struct sockaddr *)&client, &client_len)) < 0) {
            fprintf(stderr, "Can't receive datagram\n");
            exit(1);
      }

      // Modify the buffer to simulate dropped packets
      int m = 0;	// Size of modified buffer
      int i, j, r;	// i: index of original buffer
      				// j: index of new buffer
      				// r: random number deciding whether to drop packet
      char droppedBuf[MAXLEN];
      printf("HELLO WORLD!");
      for(i = 0; i < n; i++) {
      	r = 1 + (rand() % 100);	// range of 1-100
      	// if( r > DROP_RATE ) {
      	if( I%2 == 0 ) {
      		// Don't drop the packet
      		droppedBuf[j] = buf[i];
      		j++;
      	} else {
      		// By not copying over the element, we "drop" it
      		// DO NOTHING, intentionally
      	}
      }

      if (sendto(sd, buf, n, 0, 
      (struct sockaddr *)&client, client_len) != n) {
            fprintf(stderr, "Can't send datagram\n");
            exit(1);
      }
   }
   close(sd);
   return(0);
}

