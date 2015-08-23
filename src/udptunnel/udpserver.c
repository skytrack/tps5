#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h> 
#include <errno.h> 

static volatile sig_atomic_t bContinueToWork = 1;

void ExitSignalHandler(int a) {
	bContinueToWork = 0;
}

int main(int argc, char* argv[]) 
{
	int 			udp_sock, tcp_sock, client_sock, status, nfds;
	struct sockaddr_in 	server;
	fd_set 			fdReadSet;
	struct timeval 		tv;
	socklen_t		addrlen;
	unsigned char		buffer[4096];

	struct sockaddr_in 	udp_client;
	socklen_t		udp_client_len;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, ExitSignalHandler);
	signal(SIGINT, ExitSignalHandler);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(35595);

	udp_sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if (udp_sock < 0) {
		return 0;
	}

	status = bind(udp_sock, (struct sockaddr*)&server, sizeof(server));
	if (status == -1) {
		close(udp_sock);
		return 0;
	}  

	tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_sock < 0) {
		close(udp_sock);
		return 0;
	}
	
	status = 1;
	setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&status, sizeof(status));
	server.sin_port = htons(20000);
	status = bind(tcp_sock, (struct sockaddr*)&server, sizeof(server));

	if (status == -1) {
		close(udp_sock);
		close(tcp_sock);
	}

	status = listen(tcp_sock, 5);
	if (status == -1) {
		close(udp_sock);
		close(tcp_sock);
		return 0;
	}

	client_sock = -1;

	while (bContinueToWork > 0) {

		FD_ZERO(&fdReadSet);
		FD_SET(tcp_sock, &fdReadSet);
		FD_SET(udp_sock, &fdReadSet);

		nfds = 0;

		if (nfds < tcp_sock) nfds = tcp_sock;	
		if (nfds < udp_sock) nfds = udp_sock;	

		tv.tv_sec  = 1;
		tv.tv_usec = 0;

		status = select(nfds + 1, &fdReadSet, NULL, NULL, &tv);
	
		if (status > 0) {

			if (FD_ISSET(tcp_sock, &fdReadSet)) {
				
				addrlen = sizeof(server);

				client_sock = accept(tcp_sock, (struct sockaddr*)&server, &addrlen);
   			} 

			if (FD_ISSET(udp_sock, &fdReadSet)) {

				udp_client_len = sizeof(udp_client);

				status = recvfrom(udp_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&udp_client, &udp_client_len);

				if (client_sock != -1) 
					send(client_sock, buffer, status, 0);
			}

			if ((client_sock != -1)&&(FD_ISSET(client_sock, &fdReadSet))) {
			}
   		}
	}
}
