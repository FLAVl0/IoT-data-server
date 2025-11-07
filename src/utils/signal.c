#include "../include/signal_utils.h"

// Signal handler for SIGINT and SIGTERM.
// Prints a message and exits the program with the received signal number.
void signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
		printf("Received signal %d, shutting down...\n", signum);
	else
		printf("Received unexpected signal %d\n", signum);

	printf("Shutting down server...\n");
	exit(signum);
}

// Safely closes UDP and TCP sockets, prints an error message if provided,
// and calls the signal handler to terminate the program.
void safe_close_sock(struct sock_fd *sockets, char *msg)
{
	int udp_socket = sockets->udp_socket;
	int tcp_socket = sockets->tcp_socket;

	if (udp_socket >= 0)
		close(udp_socket);

	if (tcp_socket >= 0)
		close(tcp_socket);

	if (msg)
		perror(msg);

	signal_handler(SIGTERM);
}

// Safely closes the TCP listening socket, then calls safe_close_sock
// to close remaining sockets and terminate the program.
void safe_close_listen(struct sock_fd *sockets, char *msg)
{
	int listener_fd = sockets->tcp_socket;

	if (listener_fd >= 0)
		close(listener_fd);

	safe_close_sock(sockets, msg);
}
