#define SERVER_UDP_PORT         5000
#define MAXLEN                  4096
#define BUF_SIZE                1048576   /* block transfer size */
#define DEFLEN                  64
#define MAX_PRINT_BUF			1024
#define TIMEOUT_MS  			50 		// Timeout in millisec
#define MAX_RETRIES				15

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


void print_buf(char *buf, int bytes);
long delay(struct timeval t1, struct timeval t2);
int calculateNumFrames(int bytes, int data_size);
int bufferRandom(char *sbuf, int data_size);
int readFile(char *sbuf, char *filename);
int writeFile(char *sbuf, char *filename, int filesize);
void fatal(char *string);