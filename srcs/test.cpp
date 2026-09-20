#include "../includes/Client.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <cerrno>

#include <cstdlib>

std::string	intToString(int value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./server <port>" << std::endl;
        return 1;
    }

    int port = atoi(argv[1]); //have to change later for tighter checks!!!!

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Error making socket.");
        return -1;
    }

    fcntl(server_fd, F_SETFL, O_NONBLOCK); //prevent blocking

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int server_bind = bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr));

    if (server_bind == -1)
    {
        perror("Error server binding");
        close(server_fd);
        return -1;
    }

    int server_listen = listen(server_fd, 128);
    if (server_listen == -1)
    {
        std::cerr << "server listening error." << std::endl;
        close(server_fd);
        return -1;
    }

    std::vector<struct pollfd> fds;
    std::map<int, Client *> clients;

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

        for (size_t i = 0; i < fds.size();)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == server_fd)
                {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);

                    int client_fd = accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);

                    if (client_fd != -1)
                    {
                        fcntl(client_fd, F_SETFL, O_NONBLOCK);

                        const char *host = inet_ntoa(client_addr.sin_addr);

                        Client *client = new Client(client_fd, host);
			client->setNickname("Random" + intToString(std::rand() % 101));
                        clients[client_fd] = client;

                        struct pollfd client_pollfd;

                        client_pollfd.fd = client_fd;
                        client_pollfd.events = POLLIN;
                        client_pollfd.revents = 0;

                        fds.push_back(client_pollfd);

                        std::cout << "A new Client connected to: " << host << std::endl;
                    }

                    ++i;
                }
                else
                {
                    int client_fd = fds[i].fd;
                    Client *client = clients[client_fd];

                    char buffer[1024];

                    int n = recv(
                        client_fd,
                        buffer,
                        sizeof(buffer) - 1,
                        0);

                    if (n > 0)
                    {
                        buffer[n] = '\0';

                        client->appendRecvBuffer(buffer);
                        std::cout << "Client " << client->getNickname() << ": " << client->getRecvBuffer();
                        ++i;
                    }
                    else if (n == 0)
                    {
                        std::cout << "Client disconnected: " << client->getNickname() << std::endl;

                        delete client;
                        clients.erase(client_fd);

                        close(client_fd);
                        fds.erase(fds.begin() + i);
                    }
                    else
                    {
                        if (errno == EWOULDBLOCK || errno == EAGAIN)
                        {
                            ++i;
                        }
                        else
                        {
                            std::cout << "Client error: " << client_fd << std::endl;

                            delete client;
                            clients.erase(client_fd);

                            close(client_fd);
                            fds.erase(fds.begin() + i);
                        }
                    }
                }
            }
            else
            {
                ++i;
            }
        }
    }

    for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        close(it->first);
        delete it->second;
    }

    close(server_fd);
    return 0;
}
