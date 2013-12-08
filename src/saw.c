// Stop-and-Wait

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "../inc/utils.h"
#include "../inc/saw.h"


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
   uint32_t len;  // Secretly use the len property, which matches the frame, as an identifier of an ACK
};


/**
 * Implement the Stop-and-Wait ARQ protocol
 *
 * Send and receiver work on one frame at a time,
 * which in our case simply means on buf element at a time.
 *
 * 
 **/
int send_saw(int client_len, struct sockaddr_in client, int sd, char *buf, int num_frames, int data_size, int bytes, int dropRate)
{

   printf("START [S-a-W] Sending %d x %d B Frames of total size: %d...\n", num_frames, data_size, bytes);

   // Modify the buffer to simulate dropped packets
   int m = 0;  // Size of modified buffer
   int i, j, r, n;   // i: index of original buffer
               // j: index of new buffer
               // r: random number deciding whether to drop packet
   char droppedBuf[num_frames];
   struct frame a_frame;
   struct ack an_ack;
   struct  timeval         start, end;
   struct timeval timeout;
   int time_diff=0, retries;
   int bytes_sent = 0, bytes_left;

   // TIMEOUT_MS is in milliseconds
   timeout.tv_usec = 1000*TIMEOUT_MS;
   printf("   SET Socket timeout to %d ms (should be %d ms)\n", timeout.tv_usec/(1000), TIMEOUT_MS );
   // First, configure a TIMEOUT_MS
   if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, 
                  &timeout, sizeof(timeout) ) < 0){
      printf("END [FAILURE] Error setting timeout. errno: %d - %s\n", errno, strerror(errno) );
      return -1;
   }

   for(i=0, j=0; i < num_frames; i++) {

      bytes_left = bytes - bytes_sent;
      retries = 0;

      a_frame.seq = i;
      a_frame.len = min( data_size, bytes_left );
      a_frame.num_frames = num_frames;

      while(1) {
         printf("   START Send Frame: SEQ #%d, FRAMES %d, LEN %d B\n", a_frame.seq, a_frame.num_frames, a_frame.len);
         gettimeofday(&start, NULL); /* start delay measurement */

         printf("      MEMCPY %d B @ index %d\n", a_frame.len, (i*data_size) );
         memcpy(a_frame.data, (buf+(i*data_size)), a_frame.len );

         r = 1 + (rand() % 100); // range of 1-100
         if( r > dropRate ) {

            // Send the frame
            n = sendto(sd, &a_frame, sizeof(a_frame), 0, 
            (struct sockaddr *)&client, client_len );
            if(n == -1) {
               printf("   END [FAILURE] Frame send error: %d - %s\n", errno, strerror(errno));
               if(retries < MAX_RETRIES) {
                  printf("RETRY\n");
                  retries++;
                  continue;
               } else {
                  printf("   END [FAILURE] ACK maxed out retries\n");
                  return -1;
               }
            }

            /********** WAIT FOR ACK ******************************/
            printf("      START Wait for ACK #%d\n", a_frame.seq);

               n = recvfrom(sd, &an_ack, sizeof(an_ack), 0, 
                     (struct sockaddr *)&client, &client_len);
               if(n == -1) {
                  gettimeofday(&end, NULL); /* end delay measurement */

                  time_diff = delay(start, end);

                  printf("         INFO Time diff = %d\n", time_diff);
                  if( TIMEOUT_MS < time_diff ) {
                     printf(stderr, "      END [TIMEOUT] ACK receive error: %d - %s\n", errno, strerror(errno));
                     if(retries < MAX_RETRIES) {
                        printf("RETRY\n");
                        retries++;
                        continue;
                     } else {
                        printf("      END [FAILURE] ACK maxed out retries\n");
                        return -1;
                     }
                  } else {
                     printf("      END [FAILURE] ACK receive error: %d - %s\n", errno, strerror(errno));
                     if(retries < MAX_RETRIES) {
                        printf("RETRY\n");
                        retries++;
                        continue;
                     } else {
                        printf("      END [FAILURE] ACK maxed out retries\n");
                        return -1;
                     }
                  }
               }
printf(" ======> an_ack.len=%d, a_frame.len=%d\n", an_ack.len, a_frame.len );
               // Was it actually an ACK?
               if(an_ack.len == -1) {
                  if(an_ack.seq != a_frame.seq) {
                     printf("      END [FAILURE] ACK receive error: expected ACK #%d, got ACK #%d\n", a_frame.seq, an_ack.seq);
                     return -1;
                  }
               } else {
                  printf("      END [ HMM.. ] ACK receive error: expected ACK #%d, got FRAME #%d\n", a_frame.seq, an_ack.seq);
               }
            printf("      END Got ACK #%d\n", a_frame.seq);
            /******************************************************/

            // We have n fewer bytes to send
            bytes_sent += a_frame.len;

            droppedBuf[j] = buf[i];
            j++;

            printf("   END Sent frame with result: %d\n", n);

            // Successfully sent and got ACK, so go to next frame, UNLESS THIS IS THE LAST FRAME
            if(a_frame.seq+1 == num_frames) {
               // Special case -- send
            }

            break;
         } else {
            j++;
            printf("   -X- DROP (Frame #%d)\n", a_frame.seq);
            // By not copying over the element, we "drop" it
            // DO NOTHING, intentionally
         }
      } // while(1)
   } // for each frame
   printf("END Tried to send %d frames. Dropped %d frames\n", num_frames, num_frames-j);

   print_buf(buf, bytes_sent);

   return bytes_sent;
}

int receive_saw(int *client_len, struct sockaddr_in *client, int sd, char *buf, int *data_size, int dropRate)
{
   int n, i=0, seq = -1, num_frames=1, bytes_recvd=0, r;
   struct frame a_frame;
   struct ack an_ack;
   struct timeval timeout;
   int time_diff=0, retries=0;

   printf("START Receive UDP\n");

   a_frame.seq = -1;
   i=0;
   while(a_frame.seq+1 < num_frames) {

      printf("   START Receive frame (should be SEQ #%d)\n", a_frame.seq+1);
      n = recvfrom(sd, &a_frame, sizeof(a_frame), 0, 
            (struct sockaddr *)client, client_len);
      if(n == -1) {
         printf("   END [FAILURE] Frame receive error: %d - %s\n", errno, strerror(errno));
         return -1;
      }
      seq = a_frame.seq;
      *data_size = max( *data_size, a_frame.len);
      num_frames = a_frame.num_frames;


         /********** SEND ACK ******************************/
            printf("      START Send ACK #%d\n", a_frame.seq);

            an_ack.seq = a_frame.seq;
            an_ack.len = -1;

            r = 1 + (rand() % 100); // range of 1-100
            if( r > dropRate ) {

               // Send the frame
               n = sendto(sd, &an_ack, sizeof(an_ack), 0, 
                     (struct sockaddr *)client, *client_len);
               if(n == -1) {
                  printf("      END [FAILURE] ACK send error: %d - %s\n", errno, strerror(errno));
                  continue;
               }

            } else {
               printf("      -X- DROP (ACK #%d)\n", an_ack.seq);
               // If it was a real drop, we would have caught above and received a timeout
               printf("      END [TIMEOUT] ACK send error: someone wrote code to simulate dropped pkts...\n");
               continue;
            }
         printf("      END Sent ACK #%d\n", a_frame.seq);
         /******************************************************/

      printf("      MEMCPY %d B @ index %d\n", a_frame.len, (a_frame.seq*(*data_size)) );
      memcpy((buf+(a_frame.seq*(*data_size))), a_frame.data, *data_size);
      bytes_recvd += a_frame.len;
      printf("   END Received frame with result: %d, SEQ #%d, FRAMES %d, LEN %d\n", n, seq, num_frames, a_frame.len);
   }

   printf("END [SUCCESS] Receive UDP, %d x %d Frames of total size: %d\n", i, *data_size, bytes_recvd );
   print_buf(buf, bytes_recvd);

   return bytes_recvd;
}