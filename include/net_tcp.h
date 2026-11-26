#ifndef NET_TCP_H
#define NET_TCP_H

// #include "mavlink/c_library_v2-master/ardupilotmega/mavlink.h"

#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int tcp_connect(const char* ip, uint16_t port);

ssize_t tcp_send(int sock_fd, const uint8_t* buf, size_t buf_len);

ssize_t tcp_receive(int sock_fd, uint8_t* buf, size_t max_len);

void tcp_close(int sock_fd);

#endif /* NET_TCP_H */