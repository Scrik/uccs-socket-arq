#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "utils.h"


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

long delay(struct timeval t1, struct timeval t2)
{
   long d;
   d = (t2.tv_sec - t1.tv_sec) * 1000;
   d += ((t2.tv_usec - t1.tv_usec + 500) / 1000);
   return(d);
}

void fatal(char *string)
{
  printf("%s\n", string);
  exit(1);
}


int bufferRandom(char *sbuf, int data_size)
{
  int i, j;
  for (i = 0; i < data_size; i++) {
     j = (i < 26) ? i : i % 26;
     sbuf[i] = 'a' + j;
  } // construct data to send to the server
  return data_size;
}

int readFile(char *sbuf, char *filename) 
{
  printf("   START Read from file: %s\n", filename);

  int bytes, total_bytes = 0, fd;
  /* Get and return the file. */
  fd = open(filename, O_RDONLY); /* open the file */
  if (fd < 0) fatal("read open failed");

  while (1) {
    bytes = read(fd, sbuf, BUF_SIZE); /* read from file */
    if (bytes <= 0) break;     /* check for end of file */
    total_bytes += bytes;
  }
  close(fd);         /* close file */
  
  printf("   END Read %d bytes\n", total_bytes);

  return total_bytes;
}

int writeFile(char *sbuf, char *filename, int filesize)
{

  /* Go get the file and write it to a local file 
     (new or existing, overwritten) of the same name. */
  // int bytes, packets=0, fd;

  // printf("   START Write to file: %s\n", filename);

  // fd = open(filename, O_WRONLY); /* open the file */
  // if (fd < 0) fatal("write open failed");

  // // while (1) {
  //   bytes = write(fd, sbuf, BUF_SIZE);
  //   // if (bytes <= 0) break;     /* check for end of file */
  //   // packets++;
  // // }
  // close(fd);         /* close file */

  // return bytes/BUF_SIZE;

  int bytes;
  FILE *dst;
  printf("   START Write to file: %s, size: %d\n", filename, filesize);
  dst = fopen(filename, "w+");
  bytes = fwrite(sbuf, filesize*sizeof(char), 1, dst);
  fclose(dst);
  printf("   END Wrote %d bytes\n", bytes);

  return bytes;
}