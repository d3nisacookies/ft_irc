#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Error making socket.");
        return -1;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int server_bind = bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr));
    if (server_bind == -1)
    {
        perror("Error server binding");
        return -1;
    }
    int server_listen = listen(server_fd, 128);
    if (server_listen == -1)
    {
        std::cerr << "server listening error." << std::endl;
        return -1;
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), reinterpret_cast<socklen_t*>(&client_len));
    if (client_fd == -1)
    {
        std::cerr << "Client accept error." << std::endl;
        return -1;
    }

    std::cout << "Client connected!" << std::endl;
    return 0;
}