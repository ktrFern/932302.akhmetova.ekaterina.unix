#include <iostream>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

using namespace std;

volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int sig) {
    wasSigHup = 1;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
        return EXIT_FAILURE;
    }
    
    int port = atoi(argv[1]);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, SOMAXCONN);
    cout << "Server started on port " << port << endl;
    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);
    sigset_t blockedMask, origMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);
    int client_fd = -1;
    
    while (true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(listen_fd, &fds);
        int maxFd = listen_fd;
        
        if (client_fd != -1) {
            FD_SET(client_fd, &fds);
            if (client_fd > maxFd) maxFd = client_fd;
        }
        
        int ready = pselect(maxFd + 1, &fds, NULL, NULL, NULL, &origMask);
        
        if (ready == -1) {
            if (errno == EINTR) {
                if (wasSigHup) {
                    cout << "Received SIGHUP signal" << endl;
                    wasSigHup = 0;
                    continue;
                }
            } 
            else {
                perror("pselect");
                break;
            }
        }
        
        if (FD_ISSET(listen_fd, &fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_client = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
            
            if (new_client != -1) {
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
                cout << "New connection from " << client_ip << endl;
                
                if (client_fd == -1) {
                    client_fd = new_client;
                    cout << "Connection accepted" << endl;
                } 
                else {
                    cout << "Closing additional connection" << endl;
                    close(new_client);
                }
            }
        }
        
        if (client_fd != -1 && FD_ISSET(client_fd, &fds)) {
            char buffer[1024];
            ssize_t bytes = recv(client_fd, buffer, sizeof(buffer), 0);
            
            if (bytes > 0) {
                cout << "Received " << bytes << " bytes" << endl;
            } 
            else if (bytes == 0) {
                cout << "Client disconnected" << endl;
                close(client_fd);
                client_fd = -1;
            } 
            else {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("recv");
                    close(client_fd);
                    client_fd = -1;
                }
            }
        }
    }
    
    if (client_fd != -1) close(client_fd);
    close(listen_fd);
    return EXIT_SUCCESS;
}
