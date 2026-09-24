<<<<<<< Updated upstream
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
=======
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksan <ksan@student.42.sg>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/20 15:37:54 by ksan              #+#    #+#             */
/*   Updated: 2026/09/24 15:55:43 by ksan             ###   ########.sg       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

struct pollfd Server::create_fd( const int fd )
{
    struct pollfd _pollfd;
    _pollfd.fd = fd;
    _pollfd.events = POLLIN;
    all_fds.push_back(_pollfd);
}
>>>>>>> Stashed changes
