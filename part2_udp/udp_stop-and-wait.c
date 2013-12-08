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


int calculateNumFrames(int bytes, int data_size)
{
   int num_frames;
   num_frames = bytes/data_size;
   // Do we need an additional, shorter frame?
   if(num_frames*data_size < bytes)
      num_frames++;
   return num_frames;
}

void print_buf(char *buf, int bytes)
{
   printf("START Print buffer of size: %d\n", bytes);
   if(bytes > MAX_PRINT_BUF) {
      bytes = MAX_PRINT_BUF;
      printf("   TRUNCATE Buffer too long, truncate to %d B\n", bytes);
   }
   int i;
   for(i=0; i < bytes; i++) {
      printf("%c", buf[i]);
   }
   printf("\n");
   printf("END Print buffer\n");
}


/**
 * Implement the Stop-and-Wait ARQ protocol
 *
 * Send and receiver work on one frame at a time,
 * which in our case simply means on buf element at a time.
 *
 * 
 **/
int send_udp(int client_len, struct sockaddr_in client, int sd, char *buf, int num_frames, int data_size, int bytes, int dropRate)
{

   printf("START [S-a-W] Sending %d x %d B Frames of total size: %d...\n", num_frames, data_size, bytes);

   // Modify the buffer to simulate dropped packets
   int m = 0;  // Size of modified buffer
   int i, j, r, n;   // i: index of original buffer
               // j: index of new buffer
               // r: random number deciding whether to drop packet
   char droppedBuf[num_frames];
   struct frame a_frame;
   int bytes_sent = 0, bytes_left;

   for(i=0, j=0; i < num_frames; i++) {
      r = 1 + (rand() % 100); // range of 1-100

      bytes_left = bytes - bytes_sent;

      a_frame.seq = i;
      a_frame.len = min( data_size, bytes_left );
      a_frame.num_frames = num_frames;

      printf("   START Send Frame: SEQ #%d, FRAMES #%d, LEN %d B\n", a_frame.seq, a_frame.num_frames, a_frame.len);

      memcpy(a_frame.data, (buf+(i*data_size)), a_frame.len );

      if( r > dropRate ) {

         // Send the frame
         n = sendto(sd, &a_frame, sizeof(a_frame), 0, 
         (struct sockaddr *)&client, client_len );
         if(n == -1) {
            fprintf(stderr, "END [FAILURE] Frame send error: %d - %s\n", errno, strerror(errno));
            return -1;
         }
         printf("   END Sent frame with result: %d\n", n);

         // We have n fewer bytes to send
         bytes_sent += a_frame.len;

         droppedBuf[j] = buf[i];
         j++;
      } else {
         printf("DROP\n");
         // By not copying over the element, we "drop" it
         // DO NOTHING, intentionally
      }
   }
   printf("END Tried to send %d frames. Dropped %d frames\n", num_frames, num_frames-j);

   print_buf(buf, bytes_sent);

   return bytes_sent;
}

int receive_udp(int *client_len, struct sockaddr_in *client, int sd, char *buf, int *data_size)
{
   int n, i=0, seq = -1, num_frames=1, bytes_recvd=0;
   struct frame a_frame;

   printf("START Receive UDP\n");

   for(i=0; i<num_frames; i++) {
      printf("   START Receive frame %3d\n", i);
      n = recvfrom(sd, &a_frame, sizeof(a_frame), 0, 
            (struct sockaddr *)client, client_len);
      if(n == -1) {
         fprintf(stderr, "   END [FAILURE] Frame receive error: %d - %s\n", errno, strerror(errno));
         return -1;
      }
      seq = a_frame.seq;
      *data_size = max( *data_size, a_frame.len);
      num_frames = a_frame.num_frames;
      printf("   END Received frame with result: %d, SEQ #%d, FRAMES %d, LEN %d\n", n, seq, num_frames, a_frame.len);

      if(seq == -1) {
         printf("END [FAILURE] Expected SEQ #0, got SEQ #%d", seq);
         return -1;
      }

      memcpy((buf+(i*(*data_size))), a_frame.data, *data_size);

      bytes_recvd += a_frame.len;
   }

   // if ((n = recvfrom(sd, buf, MAXLEN, 0, 
   // (struct sockaddr *)&client, &client_len)) < 0) {
   //       fprintf(stderr, "END [FAILURE] Can't receive datagram\n");
   //       return -1;
   // }
   printf("END [SUCCESS] Receive UDP, %d x %d Frames of total size: %d\n", i, *data_size, bytes_recvd );

   print_buf(buf, bytes_recvd);

   return bytes_recvd;
}