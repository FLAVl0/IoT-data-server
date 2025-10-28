#ifndef SIGNAL_UTILS_H
#define SIGNAL_UTILS_H

// Include other necessary headers for this module

#include "standard.h"
#include "types.h"

// System libraries for signal handling and error reporting

#include <errno.h>
#include <signal.h>
#include <stdatomic.h>

void signal_handler(int signum);
void safe_close_sock(struct sock_fd *sockets, char *msg);
void safe_close_listen(struct sock_fd *sockets, char *msg);

#endif // SIGNAL_UTILS_H