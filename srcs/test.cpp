#include <iostream>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
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

    std::vector<struct pollfd> fds;

    struct pollfd server_pollfd;

    server_pollfd.fd = server_fd;
    server_pollfd.events = POLLIN;
    server_pollfd.revents = 0;

    fds.push_back(server_pollfd);

    while (true)
    {
        int ret = poll(fds.data(), fds.size(), -1);

        if (ret == -1)
        {
            perror("poll");
            break;
        }

        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == server_fd)
                {
                    // New client connection
                    int client_fd = accept(server_fd, NULL, NULL);

                    if (client_fd != -1)
                    {
                        struct pollfd client_pollfd;

                        client_pollfd.fd = client_fd;
                        client_pollfd.events = POLLIN;
                        client_pollfd.revents = 0;

                        fds.push_back(client_pollfd);
                    }
                }
                else
                {
                    // Existing client sent data
                    char buffer[1024];

                    int n = read(fds[i].fd, buffer, sizeof(buffer));

                    if (n > 0)
                    {
			buffer[n] = 0;
                        std::cout << "Client sent [+]: ";
			std::cout << buffer << std::endl;
                    }
                    else if (n == 0)
                    {
                        close(fds[i].fd);
			fds.erase(fds.begin() + i);
			i--;
                    }
                }
            }
        }
    }

    return 0;
}
