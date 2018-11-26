#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<error.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<errno.h>
#include<string.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Please provide the port as input.\n");
		return 0;
	}

	int tcp_server_fd, udp_server_fd;
	struct sockaddr_in tcp_server, udp_server;
	int sockaddr_len = sizeof(struct sockaddr_in);
	int tcp_data_len, udp_data_len;
	char tcp_data[1024], udp_data[1024];

	// Stuff for select
	fd_set rfds, wfds;
	struct timeval tv;
	int select_return;

	// Initialize read select
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	FD_SET(4, &rfds);

	// Initialize write select
	FD_ZERO(&wfds);
	FD_SET(3, &wfds);

	// Set up the tcp client
	if((tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(-1);
	}

	tcp_server.sin_family = AF_INET;
	tcp_server.sin_port = htons(atoi(argv[1]));
	bzero(tcp_server.sin_zero, 8);

	if(inet_pton(AF_INET, "127.0.0.1", &tcp_server.sin_addr) == -1) {
		perror("ip address");
		exit(-1);
	}

	// Set up the udp client
	if((udp_server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(-1);
	}

	printf("Started udp_server_fd %d\n", udp_server_fd);

	udp_server.sin_family = AF_INET;
	udp_server.sin_port = htons(atoi(argv[1]));
	// bzero(udp_server.sin_zero, 8);
	udp_server.sin_addr.s_addr = INADDR_ANY;

	// TCP connection
	if(connect(tcp_server_fd, (struct sockaddr *)&tcp_server, sockaddr_len) == -1)
	{
		perror("connect");
		exit(-1);
	}
	printf("Connected to tcp_server_fd %d\n", tcp_server_fd);

	// if(connect(udp_server_fd, (struct sockaddr *)&udp_server, sockaddr_len) == -1)
	// {
	// 	perror("connect");
	// 	exit(-1);
	// }

	int n = 0, len, num_fds;
	// int len = sizeof(struct sockaddr_in);

	// UDP connection
	char msg[27] = "UDP client initial message\0";
	char confirm[7] = "Confirm";

	printf("Attempting to send to UDP at %d\n", udp_server_fd);

	printf("Sockaddr_len: %d\n", sockaddr_len);

	while(n == 0) {
	printf("In UDP while loop\n");
	sendto(udp_server_fd, msg, strlen(msg), 0, (struct sockaddr *)&udp_server, sockaddr_len);
	// sendto(udp_server_fd, "abcd", strlen("abcd"), MSG_CONFIRM, (struct sockaddr *)&udp_server, sockaddr_len);
	printf("UDP sent message: %s\n", msg);

	n = recvfrom(udp_server_fd, udp_data, sizeof(udp_data), 0, (struct sockaddr *)&udp_server, &len);
	printf("Received UDP message: %s\n", udp_data);
	printf("Received num bytes: %d\n", n);

	// sendto(udp_server_fd, confirm, strlen(confirm), 0, (struct sockaddr *)&udp_server, sockaddr_len);
	// break;
	}

	printf("Confirmed connection to udp_server_fd %d\n", udp_server_fd);

	int udp_len = 1;

	while(1)
	{
		FD_SET(4, &rfds); // read from udp
		FD_SET(0, &rfds);

		FD_SET(3, &wfds);

		num_fds = select(5, &rfds, &wfds, NULL, NULL);

		// printf("Num ready descriptors: %d\n", num_fds);

		if (FD_ISSET(tcp_server_fd, &wfds) && FD_ISSET(0, &rfds)) {
			scanf("%s", tcp_data);
			// tcp_data = "a";

			send(tcp_server_fd, tcp_data, strlen(tcp_data), 0);
			printf("TCP sent message: %s\n", tcp_data);
		}

		// tcp_data_len = read(tcp_server_fd, tcp_data, 1024);
		// printf("TCP received message: %s\n", tcp_data);

		// sendto(udp_server_fd, "abcd", strlen("abcd"), MSG_CONFIRM, (struct sockaddr *)&udp_server, sockaddr_len);
		// printf("UDP sent message: abcd\n");

		// FD_SET(4, &rfds);
		if (FD_ISSET(udp_server_fd, &rfds)) {
			// printf("Preparing to recv from UDP\n");
			// n = recv(udp_server_fd, udp_data, 1024, 0);
			// n = recv(udp_server_fd, udp_data, strlen(udp_data), MSG_WAITALL);
			n = recvfrom(udp_server_fd, udp_data, sizeof(udp_data), 0, (struct sockaddr *)&udp_server, &len);
			// printf("UDP received message: %d\n", atoi(udp_data));
			printf("UDP received message: %s\n", udp_data);

			// udp_len += n;
			// printf("Received %d bytes\n", n);
		}
	}
	
	return 0;
}
