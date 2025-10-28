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

void send_file(int client_fd, const char *path, const char *content_type);
void *handle_client(void *arg);

#endif // NETWORK_H