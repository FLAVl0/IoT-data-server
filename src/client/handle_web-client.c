#include "../include/network.h"
#include "../include/types.h"

void handle_client(int client_socket)
{
	const char *bad_request =
		"HTTP/1.1 400 Bad Request\r\n"
		"Content-Length: 0\r\n"
		"Connection: close\r\n"
		"\r\n";

    struct sockaddr_in address;													   // Structure to hold client address
    socklen_t address_len = sizeof(address);									   // Length of the address structure
    if (getpeername(client_socket, (struct sockaddr *)&address, &address_len) < 0) // Get client address
    {
        perror("getpeername");
        // Let caller close the socket
        return;
    }

    char buffer[4096];	 // Larger buffer to store incoming data
    bool running = true; // Flag to control the receiving loop

    printf("Accepted connection from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    while (running)
    {
        ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0); // Receive data from client
        if (bytes_read < 0)
        {
            // Non-recoverable recv error, stop handling this client and let caller close socket
            perror("recv");
            return;
        }
        else if (bytes_read == 0)
        {
            // Peer closed connection
            printf("Client disconnected %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            return;
        }

        buffer[bytes_read] = '\0'; // Null-terminate the received data

        // Parse HTTP method and path
        char method[8], path[256];
        if (sscanf(buffer, "%7s %255s", method, path) != 2)
        {
            send(client_socket, bad_request, strlen(bad_request), 0);
            // Stop after responding
            return;
        }
        printf("Received request: %s %s\n", method, path);

        // Extract forwarded IP if present
        char *xff = strstr(buffer, "X-Forwarded-For:");
        if (xff)
        {
            char forwarded_ip[INET_ADDRSTRLEN];
            if (sscanf(xff, "X-Forwarded-For: %15s", forwarded_ip) == 1)
            {
                printf("Forwarded IP: %s\n", forwarded_ip);
            }
        }

        if (strcmp(method, "GET") == 0)
        {
            if (strcmp(path, "/") == 0)
            {
                send_file(client_socket, "www/pages/index.html", "text/html");
                // Serve one request per connection then return
                return;
            }
            else if (strncmp(path, "/styles/", 8) == 0 && strstr(path, ".css"))
            {
                char file_path[256];
                snprintf(file_path, sizeof(file_path), "www%s", path);
                send_file(client_socket, file_path, "text/css");
                return;
            }
            else if (strncmp(path, "/javascript/", 12) == 0 && strstr(path, ".js"))
            {
                char file_path[256];
                snprintf(file_path, sizeof(file_path), "www%s", path);
                send_file(client_socket, file_path, "application/javascript");
                return;
            }
            else
            {
                // Unknown GET - respond 400
                send(client_socket, bad_request, strlen(bad_request), 0);
                return;
            }
        }

        if (strcmp(method, "POST") == 0)
        {
#ifdef __linux__
            DBusError err;
            dbus_error_init(&err);
            DBusConnection *dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
            if (!dbus_conn)
            {
                fprintf(stderr, "DBus connection error: %s\n", err.message);
                dbus_error_free(&err);
                const char *response =
                    "HTTP/1.1 500 Internal Server Error\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n";
                send(client_socket, response, strlen(response), 0);
                return;
            }

            // Handle LED toggle
            if (strcmp(path, "/api/toggle-led") == 0)
            {
                char *body = strstr(buffer, "\r\n\r\n");

                if (body)
                {
                    body += 4; // Move past the header-body separator
                    printf("Toggling LED with command: %s\n", body);

                    if (write_characteristic(dbus_conn, "/org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX/service0011/char0012", body) == 0)
                    {
                        const char *response =
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Length: 0\r\n"
                            "Connection: close\r\n"
                            "\r\n";
                        send(client_socket, response, strlen(response), 0);
                    }
                    else
                    {
                        const char *response =
                            "HTTP/1.1 500 Internal Server Error\r\n"
                            "Content-Length: 0\r\n"
                            "Connection: close\r\n"
                            "\r\n";
                        send(client_socket, response, strlen(response), 0);
                    }
                    // Stop after responding
                    return;
                }
                else
                {
                    send(client_socket, bad_request, strlen(bad_request), 0);
                    return;
                }
            }
#endif
            // If POST path isn't handled, respond 400
            send(client_socket, bad_request, strlen(bad_request), 0);
            return;
        }

        // Unsupported method - respond 400
        send(client_socket, bad_request, strlen(bad_request), 0);
        return;
    }
}