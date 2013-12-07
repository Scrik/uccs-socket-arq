// Stop-and-Wait

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "udp_stop-and-wait.h"

#define SERVER_UDP_PORT    5000
#define MAXLEN             4096

/**
 * Frame 0's data will simply be an int, the number of Frames that will be sent
 **/
struct frame
{
   uint32_t seq;
   char data[MAXLEN];
};
struct ack
{
   uint32_t seq;
};

/**
 * Implement the Stop-and-Wait ARQ protocol
 *
 * Send and receiver work on one frame at a time,
 * which in our case simply means on buf element at a time.
 *
 * 
 **/
int send_udp(int client_len, struct sockaddr_in client, int sd, char *buf, int n, int dropRate)
{

   printf("START [S-a-W] Sending datagram...\n");
   printf("   CLIENT => client_len=%d, n=%d\n", client_len, n);

   // Modify the buffer to simulate dropped packets
   int m = 0;  // Size of modified buffer
   int i, j, r;   // i: index of original buffer
               // j: index of new buffer
               // r: random number deciding whether to drop packet
   char droppedBuf[n];
   struct frame a_frame;

   // printf("   START Send info packet, saying ");

   printf("   START iterating over packets! BufIn.len = %d\n", n);
   printf("      ");
   for(i=0, j=0; i < n; i++) {
      r = 1 + (rand() % 100); // range of 1-100
      printf("      FRAME %3d\n", i);

      if( r > dropRate ) {

         a_frame.seq = i;
         printf("memcpy\n");
         memcpy(a_frame.data, (buf+i), MAXLEN);
         // memcpy(a_frame.data, (buf+ci), packet.len);

         printf("send\n");
         // Send the packet
         if ( n = sendto(sd, &a_frame, sizeof(a_frame), 0, 
         (struct sockaddr *)&client, client_len)) {
               fprintf(stderr, "END [FAILURE] Frame send error\n");
               return 1;
         }

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

int receive_udp(int client_len, struct sockaddr_in client, int sd, char *buf, int n)
{
   printf("START Receive UDP\n");
   if ((n = recvfrom(sd, buf, MAXLEN, 0, 
   (struct sockaddr *)&client, &client_len)) < 0) {
         fprintf(stderr, "END [FAILURE] Can't receive datagram\n");
         return 1;
   }
   printf("END [SUCCESS] Receive UDP\n");
   return 0;
}