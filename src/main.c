/*

 * Main server file

 */

// Epoll (Linux) or event & time handler (MacOS) for handling multiple connections efficiently
#ifdef __linux__
#include <sys/epoll.h>
#endif

// Include the server header file for function declarations and necessary includes
#include "include/standard.h"
#include "include/signal_utils.h"
#include "include/network.h"
#include "include/types.h"
#include "include/db.h"

volatile sig_atomic_t keep_running = 1; // Atomic flag for signal handling

/*

 * Main function to start the server
 *
 * - Sets up signal handlers for graceful shutdown (SIGTERM, SIGINT)
 * - Initializes the database
 * - Creates UDP and TCP sockets
 * - Binds UDP socket for IoT device data (port 50000)
 * - Binds and listens on TCP socket for HTTP server (port 8080)
 * - Uses helper functions for error handling and resource cleanup

*/

#define MAX_EVENTS 64
#define PORT_UDP 50000
#define PORT_TCP 8080

int main(void)
{
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);

	init_db();

	struct sock_fd sockets;
	sockets.udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	sockets.tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (sockets.udp_socket < 0 || sockets.tcp_socket < 0)
	{
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	int udp_socket = sockets.udp_socket;
	int tcp_socket = sockets.tcp_socket;

	struct sockaddr_in udp_addr, tcp_addr;

	memset(&udp_addr, 0, sizeof(udp_addr));
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_addr.s_addr = INADDR_ANY;
	udp_addr.sin_port = htons(PORT_UDP); // Port for receiving incoming data from IoT devices

	if (bind(udp_socket, (const struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0)
	{
		perror("Bind failed");
		close(udp_socket);
		exit(EXIT_FAILURE);
	}

	memset(&tcp_addr, 0, sizeof(tcp_addr));
	tcp_addr.sin_family = AF_INET;
	tcp_addr.sin_addr.s_addr = INADDR_ANY;
	tcp_addr.sin_port = htons(PORT_TCP); // Port for HTTP server

	if (bind(tcp_socket, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0)
	{
		safe_close_sock(&sockets, "Bind failed");
	}

	if (listen(tcp_socket, 10) < 0)
	{
		safe_close_listen(&sockets, "Listen failed");
	}

#ifdef __linux__

	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	struct epoll_event event, events[MAX_EVENTS];

	event.events = EPOLLIN;
	event.data.fd = tcp_socket;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_socket, &event) == -1)
	{
		perror("epoll_ctl: tcp_socket");
		exit(EXIT_FAILURE);
	}

	event.events = EPOLLIN;
	event.data.fd = udp_socket;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udp_socket, &event) == -1)
	{
		perror("epoll_ctl: udp_socket");
		exit(EXIT_FAILURE);
	}

	while (keep_running)
	{
		int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

		for (int n = 0; n < nfds; ++n)
		{
			if (events[n].data.fd == tcp_socket)
			{
				int client_fd = accept(tcp_socket, NULL, NULL);

				if (client_fd >= 0)
				{
					// Use level-triggered reads for simplicity (EPOLLIN).
					// If you want EPOLLET you must set client_fd non-blocking and read until EAGAIN.
					event.events = EPOLLIN;
					event.data.fd = client_fd;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
					{
						perror("epoll_ctl: client_fd");
						close(client_fd);
					}
				}
			}
			else if (events[n].data.fd == udp_socket)
			{
				char buffer[1024];
				struct sockaddr_in client_addr;
				socklen_t addrlen = sizeof(client_addr);

				ssize_t recvd = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &addrlen);

				if (recvd > 0)
				{
					buffer[recvd] = '\0';
					printf("Received UDP message: %s\n", buffer);
				}
			}
			else
			{
				// Hand off the client socket to the HTTP handlerient socket to the HTTP handler
				int client_fd = events[n].data.fd;
				// Remove from epoll; handler manages the connection/closing				// Remove from epoll; handler manages the connection/closing
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);

				// handle_client will read the request and send responses the request and send responses
				handle_client(client_fd);

				// Ensure socket is closed if handler didn't close itnsure socket is closed if handler didn't close it
				close(client_fd);
			}
		}
	}

#endif

	return 0;
}
