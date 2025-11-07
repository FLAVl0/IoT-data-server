#ifndef NETWORK_H
#define NETWORK_H

#include "standard.h"

// Networking libraries

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#ifdef __linux__
#include <dbus/dbus.h>
int write_characteristic(DBusConnection *conn, const char *char_path, const char *message); // Function to handle IoT device connections with UDP
#endif

// Bluetooth libraries

void send_file(int client_fd, const char *path, const char *content_type);					 // Function to send files over TCP
void handle_client(int client_socket);														 // Function to handle web client connections with TCP

#endif // NETWORK_H