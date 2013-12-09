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

   printf("START [SAW] Sending %d x %d B Frames of total size: %d...\n", num_frames, data_size, bytes);

   // Modify the buffer to simulate dropped packets
   int m = 0;  // Size of modified buffer
   int r, n;   // i: index of original buffer
               // j: index of new buffer
               // r: random number deciding whether to drop packet
   char droppedBuf[num_frames];
   struct frame a_frame;
   struct ack an_ack;
   struct  timeval         start, end;
   struct timeval timeout;
   int time_diff=0, retries;
   int bytes_sent = 0, bytes_left;

   int mem_offset, mem_bytes;

   // TIMEOUT_MS is in milliseconds
   timeout.tv_sec = TIMEOUT;

   // Randomize it
   r = 1 + (rand() % 2);
   if( r ==0 )
      timeout.tv_sec++;
   else
      timeout.tv_sec--;

   timeout.tv_usec = 0;
   printf("   SET Socket timeout to %d ms (should be %d ms)\n", timeout.tv_sec*1000 + timeout.tv_usec/(1000), TIMEOUT*1000 );
   // First, configure a TIMEOUT_MS
   if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, 
                  &timeout, sizeof(timeout) ) < 0){
      printf("END [FAILURE] Error setting timeout. errno: %d - %s\n", errno, strerror(errno) );
      return -1;
   }

   a_frame.seq = 0;
   a_frame.num_frames = num_frames;
      bytes_left = bytes - bytes_sent;
      retries = 0;

   // for(i=0, j=0; i <= num_frames; i++) {

   //    bytes_left = bytes - bytes_sent;
   //    retries = 0;

   //    a_frame.seq = i;

      // a_frame.len = min( data_size, bytes_left );
      // a_frame.num_frames = num_frames;

      while(1) {
         if(a_frame.seq+1 == num_frames) {
            a_frame.len = bytes % data_size;
            printf("!!!! last frame, size is only: %d\n", a_frame.len);
         } else if(a_frame.seq+1 < num_frames) {
            a_frame.len = data_size;
         } else {
            a_frame.len = 0;  // FIN
         }
         printf("   START Send Frame: SEQ #%d, FRAMES %d, LEN %d B\n", a_frame.seq, a_frame.num_frames, a_frame.len);
         gettimeofday(&start, NULL); /* start delay measurement */

         if(a_frame.seq < num_frames) {
            mem_offset = a_frame.seq * data_size;
            mem_bytes = a_frame.len;
            printf("      MEMCPY %d B @ index %d\n", mem_bytes, mem_offset );
            memcpy(a_frame.data, (buf+mem_offset), mem_bytes );
         } else {
            printf("FIN!\n");
         }

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
            if(a_frame.seq < num_frames) {
               // We have n fewer bytes to send
               bytes_sent += a_frame.len;
            } else {
               printf("FIN\n");
            }

            // droppedBuf[j] = buf[i];
            // j++;

            printf("   END Sent frame with result: %d\n", n);

            // break;
         } else {
            // j++;
            printf("   -X- DROP (Frame #%d)\n", a_frame.seq);
            // By not copying over the element, we "drop" it
            // DO NOTHING, intentionally
         }

            /********** WAIT FOR ACK ******************************/
            if( a_frame.seq == num_frames ) {
               printf("      SKIPPING Wait for ACK to avoid corner case\n");
               break;

            } else {
               printf("      START Wait for ACK #%d\n", a_frame.seq);

                  n = recvfrom(sd, &an_ack, sizeof(an_ack), 0, 
                        (struct sockaddr *)&client, &client_len);
                  if(n == -1) {
                     if(a_frame.seq < num_frames) {
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
                     } else {
                        printf("      END [ IGNORE] Error receiving FINACK: %d - %s\n", errno, strerror(errno));
                     }
                  }

                  // Was it actually an ACK?
                  if(an_ack.len == -1) {
                     if(an_ack.seq != a_frame.seq) {
                        if(an_ack.seq == a_frame.seq -1 ) {
                           printf("      END [ IGNORE] ACK tells us we dropped a FRAME: expected ACK #%d, got ACK #%d\n", a_frame.seq, an_ack.seq);
                           a_frame.seq = an_ack.seq;
                        } else {
                           printf("      END [FAILURE] ACK receive error: expected ACK #%d, got ACK #%d\n", a_frame.seq, an_ack.seq);
                           return -1;
                        }
                     }
                  } else {
                     printf("      END [ HMM.. ] ACK receive error: expected ACK #%d, got FRAME #%d\n", a_frame.seq, an_ack.seq);
                  }
               printf("      END Got ACK #%d (FRAME #%d)\n", an_ack.seq, a_frame.seq++);
               retries = 0;   // restart the retry counter
               if(an_ack.seq >= num_frames) {
                  printf("FIN! #%d\n", an_ack.seq);
                  // i = a_frame.seq;
                  break;
               }
            }
            /******************************************************/

      } // while(1)
   // } // for each frame
   printf("END Tried to send %d frames.\n", num_frames);

   print_buf(buf, bytes_sent);

   return bytes_sent;
}

int receive_saw(int *client_len, struct sockaddr_in *client, int sd, char *buf, int *data_size, int dropRate)
{
   int n, i=0, seq = -1, num_frames=1, bytes_recvd=0, r, last_index_counted=-1;
   struct frame a_frame;
   struct ack an_ack;
   struct timeval timeout;
   int time_diff=0, retries=0;
   int mem_offset, mem_bytes;

   // TIMEOUT_MS is in milliseconds
   // timeout.tv_sec = 5;
   // timeout.tv_usec = 0;
   // printf("   SET Socket timeout to %d ms (should be %d ms)\n", timeout.tv_sec*1000 + timeout.tv_usec/(1000), TIMEOUT_MS );
   // // First, configure a TIMEOUT_MS
   // if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, 
   //                &timeout, sizeof(timeout) ) < 0){
   //    printf("END [FAILURE] Error setting timeout. errno: %d - %s\n", errno, strerror(errno) );
   //    return -1;
   // }

   printf("START [SAW] Receive UDP\n");

   a_frame.seq = -1;
   i=0;
   while(a_frame.seq+1 <= num_frames) {

      printf("   START Receive frame (should be SEQ #%d)\n", a_frame.seq+1);
      n = recvfrom(sd, &a_frame, sizeof(a_frame), 0, 
            (struct sockaddr *)client, client_len);
      if(n == -1) {
         printf("   END [FAILURE] Frame receive error: %d - %s\n", errno, strerror(errno));
         if(retries > MAX_RETRIES) {
            printf("MAX RETRIES EXCEEDED!\n");
            // Forget it, maybe it worked!
            break;
         }
         retries++;
         printf("   RETRIES REMAINING: %d\n", MAX_RETRIES - retries);
         continue;   // Try again
         // return -1;
      }
      if( a_frame.len == -1 ) {
         printf("   END [FAILURE] Expected Frame SEQ #%d, got ACK #%d\n", i, a_frame.seq);
         a_frame.seq = -1;
         continue;
      }
      printf("      FRAME SEQ #%d LEN %d B %d FRAMES\n", a_frame.seq, a_frame.len, a_frame.num_frames);

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

      if(a_frame.seq < num_frames) {
         printf("FRAME #%d,  last_index_counted=%d\n", a_frame.seq, last_index_counted);
         if(last_index_counted == a_frame.seq) {
            printf("      NO_memcpy I think we already added these!\n");
         } else {
            mem_offset = (a_frame.seq*(*data_size));
            mem_bytes = a_frame.len;
            printf("      MEMCPY %d B @ index %d\n", mem_bytes, mem_offset );
            memcpy((buf+mem_offset), a_frame.data, mem_bytes);
            bytes_recvd += a_frame.len;
            last_index_counted = a_frame.seq;
         }
      } else {
         printf("FIN!\n");
      }
      retries = 0;   // Forward progress! Restart the retry counter
      printf("   END Received frame with result: %d, SEQ #%d, FRAMES %d, LEN %d\n", n, seq, num_frames, a_frame.len);
   }

   printf("END [SUCCESS] Receive UDP, %d x %d Frames of total size: %d\n", i, *data_size, bytes_recvd );
   print_buf(buf, bytes_recvd);

   return bytes_recvd;
}