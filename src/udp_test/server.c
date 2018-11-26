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
#include <sys/time.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Please provide the port as input.\n");
		return 0;
	}

	int tcp_server_fd, tcp_client_fd;
	int udp_server_fd, udp_client_fd;
	struct sockaddr_in tcp_server, tcp_client, udp_server, udp_client;
	int sockaddr_len = sizeof(struct sockaddr_in);
	int tcp_len, udp_len, n;
	char tcp_data[1024], udp_data[1024];

	// Stuff for select
	fd_set rfds, wfds; // fixed_size buffer for 
	struct timeval tv;
	int select_return;

	// Initialize read select
	FD_ZERO(&rfds);
	FD_SET(3, &rfds);

	// Initialize write select
	FD_ZERO(&wfds);
	FD_SET(4, &wfds);

	tv.tv_sec = 0;
	tv.tv_usec = 100; // Wait up to 100 us for a fd to be available

	// TCP server initialization
	if((tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(-1);
	}

	tcp_server.sin_family = AF_INET;
	tcp_server.sin_addr.s_addr = INADDR_ANY;
	tcp_server.sin_port = htons(atoi(argv[1]));
	bzero(tcp_server.sin_zero, 8);

	if(bind(tcp_server_fd, (struct sockaddr *)&tcp_server, sockaddr_len) == -1)
	{
		perror("bind");
		exit(-1);
	}

	// UDP server initialization
	if((udp_server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket");
		exit(-1);
	}

	udp_server.sin_family = AF_INET;
	udp_server.sin_port = htons(atoi(argv[1]));
	udp_server.sin_addr.s_addr = INADDR_ANY;
	bzero(udp_server.sin_zero, 8);

	if(bind(udp_server_fd, (struct sockaddr *)&udp_server, sockaddr_len) == -1)
	{
		perror("bind");
		exit(-1);
	}

	// Listen on TCP
	if(listen(tcp_server_fd, 5) == -1)
	{
		perror("listen");
		exit(-1);
	}

	int num_fds;


	printf("Listening on tcp_server_fd %d\n", tcp_server_fd);
	// Accept a tcp client first
	while(1)
	{
		if((tcp_client_fd = accept(tcp_server_fd, (struct sockaddr *)&tcp_client, &sockaddr_len)) == -1)
		{
			perror("accept");
			exit(-1);
		}

		printf("New tcp_client from port %d and IP %s\n", ntohs(tcp_client.sin_port), inet_ntoa(tcp_client.sin_addr));
		break;
	}

	// Accept a UDP message to discover the client (blocking)
	
	char msg[21] = "Hello from UDP server";

	printf("Attempting to receive from UDP client at %d.\n", udp_server_fd);

	udp_len = sizeof(struct sockaddr_in);
	n = 0;

	while(n == 0) {
	n = recvfrom(udp_server_fd, udp_data, sizeof(udp_data), 0, (struct sockaddr *)&udp_client, &udp_len);
	printf("UDP received mesg: %s\n", udp_data);

	printf("Sending message back to UDP client: %s\n", msg);

	sendto(udp_server_fd, msg, strlen(msg), 0, (struct sockaddr *)&udp_client, sockaddr_len);
	}

	printf("Sending on udp_server_fd %d\n", udp_server_fd);

	int count = 0;
	tcp_len = 1;
	int result;
	int send_size = 0;

	while(1)
	{
		FD_SET(tcp_client_fd, &rfds);
		FD_SET(udp_server_fd, &wfds);

		num_fds = select(tcp_client_fd + 1, &rfds, &wfds, NULL, NULL);

		// printf("Num ready descriptors: %d\n", num_fds);

		// if (tcp_len)
		if (FD_ISSET(tcp_client_fd, &rfds))
		{

			// tcp_len = recv(tcp_client_fd, tcp_data, 1024, 0);
			result = read(tcp_client_fd, tcp_data + tcp_len, 1024);

			if (result == 0) {
				printf("Socket is dead. I no longer have a reason to live.\n");	
				close(tcp_server_fd);
				exit(1);
			}

			printf("TCP received mesg: %s\n", tcp_data + tcp_len);

			tcp_len += result;

			// if(tcp_len)
			// {
			// 	send(tcp_client_fd, tcp_data, tcp_len, 0);
			// 	tcp_data[tcp_len] = '\0';
			// 	printf("TCP sent mesg: %s\n", tcp_data);
			// }
		} // else {
			// printf("Client disconnected\n");
			// close(tcp_client_fd);	

		// UDP recv
		// n = recvfrom(udp_server_fd, udp_data, 1024, MSG_WAITALL, (struct sockaddr *)&udp_client, &udp_len);
		// printf("UDP received mesg: %s\n", udp_data);

		// if(n > 0) 
		// {
			// udp_data[n] = '\0';


		if (FD_ISSET(udp_server_fd, &wfds) && count % 100000 == 0) {
			send_size = snprintf(udp_data, sizeof(udp_data), "%d", count);
			printf("UDP sent bytes: %d\n", send_size);
			sendto(udp_server_fd, udp_data, send_size, 0, (struct sockaddr *)&udp_client, udp_len);
			printf("UDP sent mesg from %d: %s\n", udp_server_fd, udp_data);
		}
		count++;
		// }

	}
	
	return 0;
}
