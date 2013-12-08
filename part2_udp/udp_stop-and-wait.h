#define SERVER_UDP_PORT         5000
#define MAXLEN                  4096
#define BUF_SIZE                1048576   /* block transfer size */
#define DEFLEN                  64
#define MAX_PRINT_BUF			1024
#define TIMEOUT_MS  			50 		// Timeout in millisec

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int send_udp(int client_len, struct sockaddr_in client, int sd, 
				char *buf, int num_frames, int data_size, int bytes, int dropRate);
int receive_udp(int *client_len, struct sockaddr_in *client, int sd, 
				char *buf, int *data_size, int dropRate);
int calculateNumFrames(int bytes, int data_size);
