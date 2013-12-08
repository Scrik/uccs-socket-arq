int send_gbn(int client_len, struct sockaddr_in client, int sd, 
				char *buf, int num_frames, int data_size, int bytes, int dropRate);
int receive_gbn(int *client_len, struct sockaddr_in *client, int sd, 
				char *buf, int *data_size, int dropRate);
