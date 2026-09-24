#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

Server::Server(int port, const std::string& password) 
    : _port(port), _password(password) {}

Server::~Server() {}

void Server::addClient(Client* client)
{
    // need valiation
    _clients.push_back(client);
}

void Server::addChannel(std::string name, Channel* channel)
{
    _channels.insert(std::make_pair(name, channel));
}
