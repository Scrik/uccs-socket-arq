#define SERVER_UDP_PORT         5000
#define MAXLEN                  4096
#define BUF_SIZE                1048576   /* block transfer size */
#define DEFLEN                  64
#define MAX_PRINT_BUF			1024
#define TIMEOUT 				2		// Timeout in seconds
#define TIMEOUT_MS  			50 		// Timeout in millisec
#define GBN_TIMER     			1 		// Time after which to process ACKs
#define GBN_TIMEOUT_MS			500		// Timeout (ms) for each datagram (should be short)
#define GBN_WINDOW_SIZE			5 		// How many outstanding frames are allowed?
#define MAX_RETRIES				20

// Id of protocols
#define STOP_AND_WAIT			1
#define GO_BACK_N				2
#define SELECTIVE_REPEAT		3

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