// Go-Back-N

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "../inc/utils.h"
#include "../inc/gbn.h"


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
   uint32_t bytes;
};


/**
 * Implement the Stop-and-Wait ARQ protocol
 *
 * Send and receiver work on one frame at a time,
 * which in our case simply means on buf element at a time.
 *
 * 
 **/
int send_gbn(int client_len, struct sockaddr_in client, int sd, char *buf, int num_frames, int data_size, int bytes, int dropRate)
{

   printf("START [GBN] Sending %d x %d B Frames of total size: %d...\n", num_frames, data_size, bytes);

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
   int bytes_acked = 0, bytes_left=bytes, last_ack = 0, elapsed;

   // First FRAME will be SEQ #0
   a_frame.seq=0;


   // GBN_TIMEOUT_MS is in milliseconds
   timeout.tv_usec = 1000*GBN_TIMEOUT_MS;
   printf("   SET Socket timeout to %d ms (should be %d ms)\n", timeout.tv_usec/(1000), GBN_TIMEOUT_MS );
   // First, configure a GBN_TIMEOUT_MS
   if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, 
                  &timeout, sizeof(timeout) ) < 0){
      printf("END [FAILURE] Error setting timeout. errno: %d - %s\n", errno, strerror(errno) );
      return -1;
   }

   // Iterate until the entire file is sent
   while(1) {

      /***********************************************************************************************
       * Establish a time after which we will process the ACKs
       ***/
      gettimeofday(&start, NULL); /* start delay measurement */

      /***********************************************************************************************
       * Send a bunch of frames, and also receive ACKs
       ***/
       while(1) {

         /********************************************************************************************
          * Send a frame
          ***/
         if(   (a_frame.seq < num_frames) &&                // Don't send more than we want to send
               (a_frame.seq - last_ack) < GBN_WINDOW_SIZE   // Don't send more than the window allows
            )
         {
            a_frame.len = min( data_size, bytes_left );
            a_frame.num_frames = num_frames;

            printf("   START Send Frame: SEQ #%d, FRAMES %d, LEN %d B\n", a_frame.seq, a_frame.num_frames, a_frame.len);

            printf("      MEMCPY %d B @ index %d\n", a_frame.len, (a_frame.seq*data_size) );
            memcpy(a_frame.data, (buf+(a_frame.seq*data_size)), a_frame.len );

            r = 1 + (rand() % 100); // range of 1-100
            if( r > dropRate ) {
               n = sendto(sd, &a_frame, sizeof(a_frame), 0, 
               (struct sockaddr *)&client, client_len );
               if(n == -1) {
                  printf("   END [FAILURE] Frame send error: %d - %s\n", errno, strerror(errno));
                  // It happens, just move on...
               } else {
                  // Might have worked! Of course we wouldn't know if a real ntwk drop occurs, 
                  // but that's what ACKs are for, after all
                  a_frame.seq++;
                  printf("   END [SUCCESS] Sent frame with result: %d\n", n);
               }
            } else {
               j++;
               printf("   END [DROPPED] Dropped (Frame #%d)\n", a_frame.seq);
               // By not copying over the element, we "drop" it
               // DO NOTHING, intentionally
            }
         } else {
            printf("   NOOP Waiting for ACKs before sending more FRAMES\n");
            // We've sent all the frames, but still need to collect ACKs
            // DO NOTHING HERE, and just fall through to the "Check for an ACK" code below
         }


         /********************************************************************************************
          * Check for an ACK
          ***/
          printf("   START Check for ACK #%d\n", last_ack+1);

          n = recvfrom(sd, &an_ack, sizeof(an_ack), 0, 
                (struct sockaddr *)&client, &client_len);
          if(n == -1) {
             printf("   END [ IGNORE] ACK receive error: %d - %s\n", errno, strerror(errno));
             // Errors happen, just move on and don't count this as a recvd ACK
          } else {
             if( an_ack.seq == last_ack+1 ) {
                // Received the ACK in order, ALL IS GOOD
                last_ack = an_ack.seq;

                // ACKs include the number of bytes they are ACK'ing
                bytes_acked += an_ack.bytes;
 
                printf("   END [SUCCESS] Got ACK #%d\n", an_ack.seq);
             } else {
                // Received ACK out of order -- we probably missed an ACK, or the receiver missed a frame
                printf("   END [ IGNORE] Expected ACK #%d, got ACK #%d\n", last_ack+1, an_ack.seq);
                // DO NOTHING, and fall through, letting the "interrupt" logic handle this issue
             }
          }


         /********************************************************************************************
          * Timeout "interrupt" - i.e. poll the time and compare to start
          ***/
         gettimeofday(&end, NULL); /* end delay measurement */
         elapsed = delay(start, end);  // Get the diff in seconds

         printf("   TIMER GBN_TIMER=%d s, elapsed=%d s\n", GBN_TIMER, elapsed);
         printf("      INFO Start time: (%d s + %d ms) = %f s\n", start.tv_sec, start.tv_usec+500, 
            ( start.tv_sec + (start.tv_usec+500)/1000 ) );
         printf("      INFO End time: (%d s + %d ms) = %f s\n", end.tv_sec, end.tv_usec+500, 
            ( end.tv_sec + (end.tv_usec+500)/1000 ) );
         if(elapsed > GBN_TIMER) {
            // Timer expired! Get out of here and fall through to GBN processing
            break;
         }
       } // inner loop


      /***********************************************************************************************
       * Time expired, so process ACKs and decide if we need to go back, and if so what's N
       ***/
       if( last_ack == num_frames ) {
         // The ACK SEQ #s are always the *next* frame they want. 
         // If the next frame it wants is one past the total, that means it's time to send a "FIN",
         // aka an empty FRAME
         a_frame.seq = num_frames;
         a_frame.len = -1;
         printf("   START Send FIN #%d\n", a_frame.seq);
         n = sendto(sd, &a_frame, sizeof(a_frame), 0, 
         (struct sockaddr *)&client, client_len );
         if(n == -1) {
            // Error on FIN... Loop, and assume this will be run again.
         } else {
            printf("   END [SUCCESS] Sent FIN\n");
            // We don't care if they ACK the FIN, so just break
            break;
         }
       } else {
         // More FRAMES to send. Just loop...
       }

   }  // outer loop

   printf("END [SUCCES] GBN Sent %d frames, with %d B ACK'ed\n", num_frames, bytes_acked);
   return bytes_acked;
}

int receive_gbn(int *client_len, struct sockaddr_in *client, int sd, char *buf, int *data_size, int dropRate)
{
   int n, i=0, seq = -1, num_frames=1, bytes_recvd=0, r;
   struct frame a_frame;
   struct ack an_ack;
   struct timeval timeout;
   int time_diff=0, retries=0;

   timeout.tv_usec=0;

   // This will always have the next Frame needed
   an_ack.seq = 0;
   an_ack.len = -1;

   printf("START [GBN] Receive UDP\n");

   /***********************************************************************************************
    * Receiver is dumb... He knows what FRAME he needs next, and will only ACK if it's that one
    ***/
    while(1) {
      /********************************************************************************************
       * Check for a FRAME
       ***/
      printf("   START Receive frame (should be SEQ #%d)\n", an_ack.seq);
      n = recvfrom(sd, &a_frame, sizeof(a_frame), 0, 
            (struct sockaddr *)client, client_len);
      if(n == -1) {
         printf("   END [FAILURE] Frame receive error: %d - %s\n", errno, strerror(errno));
         
      } else {

         /********************************************
          * Only set the timeout once, and make sure to set it
          * after we receive the first FRAME
          */
         if(timeout.tv_usec == 0) {
            // GBN_TIMEOUT_MS is in milliseconds
            timeout.tv_usec = 1000*GBN_TIMEOUT_MS;
            printf("   SET Socket timeout to %d ms (should be %d ms)\n", timeout.tv_usec/(1000), GBN_TIMEOUT_MS );
            // First, configure a GBN_TIMEOUT_MS
            if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, 
                           &timeout, sizeof(timeout) ) < 0){
               printf("END [FAILURE] Error setting timeout. errno: %d - %s\n", errno, strerror(errno) );
               return -1;
            }
         }

         // Got the FRAME
         if(a_frame.seq == an_ack.seq) {
            // Its the one we want!
            
            *data_size = max( *data_size, a_frame.len );
            num_frames = a_frame.num_frames;

            if(a_frame.len == -1) {
               // FIN
               printf("   END [SUCCESS] Received FIN #%d, FRAMES %d, LEN %d B\n", a_frame.seq, a_frame.num_frames, a_frame.len);
               an_ack.bytes = a_frame.len;
               an_ack.seq++;
            } else {
               // FRAME
               // Store the data
               printf("      MEMCPY %d B @ index %d\n", a_frame.len, (a_frame.seq*(*data_size)) );
               memcpy((buf+(a_frame.seq*(*data_size))), a_frame.data, a_frame.len);

               an_ack.bytes = a_frame.len;
               an_ack.seq++;
               bytes_recvd += a_frame.len;
               printf("   END [SUCCESS] Received Frame: SEQ #%d, FRAMES %d, LEN %d B\n", a_frame.seq, a_frame.num_frames, a_frame.len);
            }

            // Fall through to the ACK code block
         } else {
            printf("   END [FAILURE] Received frame SEQ #%d, expecting SEQ #%d\n", a_frame.seq, an_ack.seq);
            // Just fall through and try to receive the next Frame... the sender will figure it out eventually
            continue;
         }
      }

      /********************************************************************************************
       * Send an ACK
       ***/
       printf("   START Send ACK #%d\n", an_ack.seq);
       // Randomly "drop" it
        r = 1 + (rand() % 100); // range of 1-100
        if( r > dropRate ) {
           // Send the ACK
           n = sendto(sd, &an_ack, sizeof(an_ack), 0, 
                 (struct sockaddr *)client, *client_len);
           if(n == -1) {
              printf("   END [FAILURE] ACK send error: %d - %s\n", errno, strerror(errno));
              // Errors happen... let the sender figure it out, and fall through to bottom of loop
           } else {
              printf("   END [SUCCESS] Sent ACK #%d\n", an_ack.seq);
           }
        } else {
           printf("   END [DROPPED] Dropped (Frame #%d)\n", an_ack.seq);
           // Errors happen... let the sender figure it out, and fall through to bottom of loop
        }

      /********************************************************************************************
       * Check end condition (the sending of a final, "FIN" frame)
       ***/
       printf("   DONE? an_ack.seq=%d > %d=num_frames\n", (an_ack.seq), num_frames);
       if(an_ack.seq > num_frames) {
         break;
       }
    } // loop

   // Unset the timeout for the next time we want to blockingly wait for a client
   timeout.tv_usec = 0;
   printf("   SET Socket timeout to %d ms (should be %d ms)\n", timeout.tv_usec/(1000), 0 );
   if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, 
                  &timeout, sizeof(timeout) ) < 0){
      printf("END [FAILURE] Error setting timeout. errno: %d - %s\n", errno, strerror(errno) );
      return -1;
   }

   printf("END [SUCCES] GBN Received %d frames, with total size %d B\n", num_frames, bytes_recvd);
   return bytes_recvd;
}