// Stop-and-Wait

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "udp_stop-and-wait.h"

#define SERVER_UDP_PORT    5000
#define MAXLEN             1024

/**
 * Frame 0's data will simply be an int, the number of Frames that will be sent
 **/
struct frame
{
   uint32_t seq;
   uint32_t len;
   uint32_t num_frames;
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
int send_udp(int client_len, struct sockaddr_in client, int sd, char *buf, int num_frames, int data_size, int dropRate)
{

   printf("START [S-a-W] Sending %d Frames...\n", num_frames);

   // Modify the buffer to simulate dropped packets
   int m = 0;  // Size of modified buffer
   int i, j, r, n;   // i: index of original buffer
               // j: index of new buffer
               // r: random number deciding whether to drop packet
   char droppedBuf[num_frames];
   struct frame a_frame;

   for(i=0, j=0; i < num_frames; i++) {
      r = 1 + (rand() % 100); // range of 1-100
      printf("   START Send frame %3d\n", i);

      a_frame.seq = i;
      a_frame.len = data_size;
      a_frame.num_frames = num_frames;
      memcpy(a_frame.data, (buf+(i*data_size)), data_size);

      if( r > dropRate ) {

         // Send the frame
         n = sendto(sd, &a_frame, sizeof(a_frame), 0, 
         (struct sockaddr *)&client, client_len);
         if(n == -1) {
            fprintf(stderr, "END [FAILURE] Frame send error: %d\n", errno);
            return 1;
         }
         printf("   END Sent frame with result: %d\n", n);

         droppedBuf[j] = buf[i];
         j++;
      } else {
         printf("DROP\n");
         // By not copying over the element, we "drop" it
         // DO NOTHING, intentionally
      }
   }
   printf("\n   END iterating over frames. Dropped %d frames\n", num_frames-j);

   print_buf(buf, data_size, num_frames);

   // if (sendto(sd, droppedBuf, j, 0, 
   // (struct sockaddr *)&client, client_len) != n) {
   //       fprintf(stderr, "END [FAILURE] Can't send datagram\n");
   //       return 1;
   // } else {
   //    printf("END [SUCCESS] Datagram sent\n");
   //    return 0;
   // }
}

int receive_udp(int client_len, struct sockaddr_in client, int sd, char *buf)
{
   int n, i=0, seq = -1, num_frames=1, data_size;
   struct frame a_frame;

   printf("START Receive UDP\n");

   for(i=0; i<num_frames; i++) {
      printf("   START Receive frame %3d\n", i);
      n = recvfrom(sd, &a_frame, sizeof(a_frame), 0, 
            (struct sockaddr *)&client, &client_len);
      if(n == -1) {
         fprintf(stderr, "   END [FAILURE] Frame receive error: %d\n", errno);
         return -1;
      }
      seq = a_frame.seq;
      data_size = a_frame.len;
      num_frames = a_frame.num_frames;
      printf("   END Received frame with result: %d, SEQ #%d, FRAMS %d, LEN %d\n", n, seq, num_frames, data_size);

      if(seq == -1) {
         printf("END [FAILURE] Expected SEQ #0, got SEQ #%d", seq);
         return -1;
      }

      memcpy((buf+(i*data_size)), a_frame.data, data_size);
   }

   // if ((n = recvfrom(sd, buf, MAXLEN, 0, 
   // (struct sockaddr *)&client, &client_len)) < 0) {
   //       fprintf(stderr, "END [FAILURE] Can't receive datagram\n");
   //       return -1;
   // }
   printf("END [SUCCESS] Receive UDP, %d Frames\n", i);

   print_buf(buf, data_size, num_frames);

   return i;
}

void print_buf(char *buf, int data_size, int num_frames)
{
   printf("START Print buffer\n");
   int i;
   for(i=0; i < data_size*num_frames; i++) {
      printf("%c", buf[i]);
   }
   printf("\n");
   printf("END Print buffer\n");
}