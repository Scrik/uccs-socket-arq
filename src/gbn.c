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
   int bytes_sent = 0, bytes_left;



   printf("END [FAILURE] GBN not implemented\n");

   return -1;
}

int receive_gbn(int *client_len, struct sockaddr_in *client, int sd, char *buf, int *data_size, int dropRate)
{
   int n, i=0, seq = -1, num_frames=1, bytes_recvd=0, r;
   struct frame a_frame;
   struct ack an_ack;
   struct timeval timeout;
   int time_diff=0, retries=0;

   printf("START [GBN] Receive UDP\n");

   printf("END [FAILURE] GBN not implemented\n");

   return -1;
}