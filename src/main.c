/*

 * Main server file

 */

// Libraries to handle signal and atomic operations for Docker compatibility and graceful shutdown
#include <signal.h>
#include <stdatomic.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

volatile sig_atomic_t keep_running = 1; // Atomic flag for signal handling

// Include the server header file for function declarations and necessary includes
#include "server.h"

/*

 * Main function to start the server
 * 
 * - Sets up signal handlers for graceful shutdown (SIGTERM, SIGINT)
 * - Initializes the database
 * - Creates UDP and TCP sockets
 * - Binds UDP socket for IoT device data (port 50000)
 * - Binds and listens on TCP socket for HTTP server (port 8080)
 * - Uses helper functions for error handling and resource cleanup
 *
*/

int main(void)
{
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    init_db();

    struct sock_fd sockets = {
        .udp_socket = socket(AF_INET, SOCK_DGRAM, 0), // UDP socket for receiving messages (IoT devices)
        .tcp_socket = socket(AF_INET, SOCK_STREAM, 0) // TCP socket for client connections (HTTP server)
    };

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
    udp_addr.sin_port = htons(50000); // Port for receiving incoming data from IoT devices

    if (bind(udp_socket, (const struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0)
    {
        perror("Bind failed");
        close(udp_socket);
        exit(EXIT_FAILURE);
    }

    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(8080); // Port for HTTP server

    if (bind(tcp_socket, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0)
        safe_close(&sockets, "Bind failed");

    if (listen(tcp_socket, 10) < 0)
        safe_close_listen(&sockets, "Listen failed");

    return 0;
}
