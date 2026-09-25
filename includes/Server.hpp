#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <cerrno>
#include <exception>
#include <cctype>

#include "Client.hpp"
#include "Channel.hpp"
#define WELCOME "Welcome to IRC server\n"
class Server
{
    private:
        int _port;
        int _server_fd;
        std::vector<struct pollfd> _all_fds;
        const std::string _password;
        std::map<int, Client *> _clients;
        std::map<std::string, Channel *> _channels;
    public:
        Server(const std::string& port, const std::string& password);
        static int parsePort(const std::string& port);
        bool    start( void );
        ~Server(); // server owns client and channel so need to destroy them

        //Socket 
        bool    server_listen( void );
        void genNewPollfd( const int client_fd );
        void    wait_poll( void );
        bool    bind_socket( void );
        void    ValidateNewClient( void );
        // Client management
        void handleClient( const int client_fd , size_t &index );
        void removeClient(Client* client); // leave all channel, remove client from all channel, remove from server, destroy client
        Client* findClientFd(int fd);
        Client* findClientNickname(std::string name); // client cannot have same nickName
 
        // Channel management
        void addChannel(std::string name, Channel* channel);
        void removeChannel(Channel* channel); // if empty, remove channel
        Channel* findChannel(std::string name);


        const int& getPort() const;
        const std::string& getPwd() const;

        void setPwd(std::string password);

        class InvalidPortException : public std::exception
        {
            public:
                virtual const char* what() const throw();
        };
        class InvalidPasswordException : public std::exception 
        {
            public:
                virtual const char* what() const throw();
        };
};


