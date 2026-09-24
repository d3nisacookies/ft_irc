#pragma once

#include <vector>
#include <map>

#include "Client.hpp"
#include "Channel.hpp"

class Server
{
    private:
        int _port;
        std::string _password;

        std::vector<Client *> _clients;
        std::map<std::string, Channel *> _channels;
    public:
        Server(int port, const std::string& password);
        ~Server(); // server owns client and channel so need to destroy them

        // Client management
        void addClient(Client* client);
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
};


