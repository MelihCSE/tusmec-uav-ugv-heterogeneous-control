#include "net_tcp.h"

int tcp_connect(const char* ip, uint16_t port){
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0){
        perror("Error: open socket\n");
        return -1;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if(inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0){
        perror("Error: Invalid ip address\n");
        close(sock_fd);
        return -1;
    }

    // Connecting ArduPilot-SITL TCP Server
    if(connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Error: Unsuccesful TCP connection\n");
        close(sock_fd);
        return -1;
    }

    // Set the socket Non-Blocking Mode
    int flags = fcntl(sock_fd, F_GETFL, 0);
    if(flags < 0 || fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK) < 0){
        perror("Error: Non-blocking mode.\n");
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}


ssize_t tcp_send(int sock_fd, const uint8_t* buf, size_t buf_len){
    
    if(sock_fd < 0 || buf == NULL || buf_len == 0){
        return -1;
    }
    
    ssize_t sent = send(sock_fd, buf, buf_len, 0);
    if(sent < 0){
        // If sent is temporary full return it as not an error
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            return 0;
        }
        perror("Error: TCP send.\n");
        return -1;
    }
    
    return sent;
}

ssize_t tcp_receive(int sock_fd, uint8_t* buf, size_t max_len){
    if(sock_fd < 0 || buf == NULL || max_len == 0){
        return -1;
    }

    ssize_t received = recv(sock_fd, buf, max_len, 0);
    if(received < 0){
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            return 0;
        }
        perror("Error: TCP receive.\n");
        return -1;
    }

    // Disconnected from ArduPilot
    if(received == 0){
        return -1;
    }

    return received;
}

void tcp_close(int sock_fd){
    if(sock_fd >= 0){
        close(sock_fd);
    }
}