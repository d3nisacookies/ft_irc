  #include "CommandHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <signal.h>
#include <csignal>
#include <strings.h>
#include <cstring>
#include <cerrno>

volatile sig_atomic_t g_running = 1;

static void handleSignal(int num)
{
    (void)num;
    g_running = 0;
}

const char* Server::InvalidPortException::what() const throw()
{
    return ("Invalid Port: must be a number between 1024 and 65535");
}

const char* Server::InvalidPasswordException::what() const throw()
{
    return ("Invalid Password: Use a better one.");
}

int Server::parsePort(const std::string& port)
{
    long value = 0;
    if (port.empty())
        throw InvalidPortException();
    for (std::size_t i = 0; i < port.size(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(port[i])))
            throw InvalidPortException();
        value =  value * 10 + (port[i] - '0');
        if (value > 65535)
            throw InvalidPortException();
    }
    if (value < 1024)
        throw InvalidPortException();
    return static_cast<int>(value);
}

Server::Server(const std::string& port, const std::string& password) 
    : _port(parsePort(port)), _server_fd(-1), _password(password)
{
    if (password.empty())
        throw InvalidPasswordException();
    for (size_t i = 0; i < password.size(); ++i)
    {
        if (std::isspace(static_cast<unsigned char>(password[i])) || std::iscntrl(static_cast<unsigned char>(password[i])))
            throw InvalidPasswordException();
    }
}

std::string	intToString(int value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

void    Server::genNewPollfd( const int client_fd )
{
    struct pollfd new_pollfd;
    new_pollfd.fd = client_fd;
    new_pollfd.events = POLLIN;
    new_pollfd.revents = 0;
    _all_fds.push_back(new_pollfd);
}

bool    Server::bind_socket( void )
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1)
    {
        perror("Error making socket.");
        return false;
    }
    fcntl(_server_fd, F_SETFL, O_NONBLOCK); //prevent blocking

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int server_bind = bind(_server_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr));

    if (server_bind == -1)
    {
        perror("Error server binding");
        close(_server_fd);
        return false;
    }
    return true;
}

bool    Server::server_listen( void )
{
    int ret_code = listen(_server_fd, 128);
    if (ret_code == -1)
    {
        std::cerr << "server listening error." << std::endl;
        close(_server_fd);
        return 0;
    }
    genNewPollfd(_server_fd);
    return 1;
}

bool Server::nicknameExist(const std::string& new_nick)
{
    for (std::map<std::string, Client*>::const_iterator it =
            _clients_byNickname.begin(); it != _clients_byNickname.end();
            ++it)
    {
        if (strcasecmp(it->first.c_str(), new_nick.c_str()) == 0)
            return true;
    }

    return false;
}

void    Server::ValidateNewClient( void )
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(_server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
    if (client_fd != -1)
    {
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        const char *host = inet_ntoa(client_addr.sin_addr);
        Client *client = new Client(client_fd, host);
        _clients[client_fd] = client;
        genNewPollfd( client_fd );
    } 
}

void    Server::removeClient(int fd)
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end() || it->second == NULL)
    {
        std::cout << "Client does not exist" << std::endl;
        return ;
    }
    Client  *client = it->second;
    std::vector<Channel *>channels = client->getChannels();
    for(std::vector<Channel *>::iterator ch = channels.begin() ; ch != channels.end(); ++ch)
    {
        (*ch)->removeMember(client);
        client->removeChannel(*ch);
        if ((*ch) ->getMembers().empty())
            removeChannel(*ch);
    }
    if (client->hasNickname())
        _clients_byNickname.erase(client->getNickname());
    _clients.erase(it);
    delete client;
}

void    Server::disconnect(int fd, size_t index)
{
    removeClient(fd);
    close(fd);
    _all_fds.erase(_all_fds.begin() + index);
}

void    Server::handleClient( const int client_fd , size_t &index)
{
    char buffer[1024];
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n == 0)
    {
        if (_clients[client_fd]->hasNickname())
            std::cout << _clients[client_fd]->getNickname() << " just disconnected." << std::endl;
        else
            std::cout << _clients[client_fd]->getFd() << " has disconnected." << std::endl; 
        disconnect(client_fd, index);
    }
    else if (n > 0)
    {
        Client* client = _clients[client_fd];
        client->appendRecvBuffer(std::string(buffer, n));

        CommandHandler handler(this);
        std::string line;
        while (client->extractLine(line))
        {
            IRCMessage msg(line);
            std::vector<Response> responses = handler.processCommand(client, &msg);

            for (size_t i = 0; i < responses.size(); ++i)
            {
                const Response& r = responses[i];
                r.destination->appendSendBuffer(r.message); // sent later, when poll() reports POLLOUT
            }
            if (client->isQuitting())
                break;
        }
        // a quitting client is removed once its output has been flushed (flushClient)
        ++index;
    }
    else
    {
        std::cout << "Client error: " << client_fd << std::endl;
        disconnect(client_fd, index);
    }
}

// One send() per POLLOUT, never in a loop. Returns false when the client must be removed.
bool    Server::flushClient(int fd)
{
    Client* client = _clients[fd];
    const std::string& buf = client->getSendBuffer();

    int n = send(fd, buf.c_str(), buf.size(), 0);
    if (n <= 0)
        return false;
    client->consumeSendBuffer(n);
    if (client->isQuitting() && client->getSendBuffer().empty())
        return false;
    return true;
}

void    Server::wait_poll( void )
{
    while (g_running)
    {
        // only ask for POLLOUT while a client has something waiting to be sent
        for (size_t i = 0; i < _all_fds.size(); ++i)
        {
            Client* client = findClientFd(_all_fds[i].fd);   // NULL for the listening socket
            _all_fds[i].events = POLLIN;
            if (client && !client->getSendBuffer().empty())
                _all_fds[i].events |= POLLOUT;
            if (client && client->isQuitting())
                _all_fds[i].events = POLLOUT;                // stop reading from it
        }

        int ret = poll(_all_fds.data(), _all_fds.size(), 1000);
        if (ret == -1)
        {
            if (!g_running)
                continue;
            perror("poll");
            break;
        }
        for (size_t i = 0; i < _all_fds.size();)
        {
            int fd = _all_fds[i].fd;
            short rev = _all_fds[i].revents;

            if (rev & POLLOUT)
            {
                if (!flushClient(fd))
                {
                    disconnect(fd, i);
                    continue;                // slot i was erased: do not ++i
                }
            }
            if ((rev & (POLLHUP | POLLERR | POLLNVAL)) && !(rev & POLLIN))
            {
                disconnect(fd, i);
                continue;
            }
            if (rev & POLLIN)
            {
                if ( _all_fds[i].fd == _server_fd)
                {
                    ValidateNewClient();
                    ++i;
                }
                else
                    handleClient(_all_fds[i].fd, i);
            }
            else
                ++i;
        }

    }
    
}

void Server::addChannel(std::string name, Channel* channel)
{
    _channels.insert(std::make_pair(name, channel));
}



static void installSignalHandlers()
{
    struct sigaction sa;

    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // a send() to a closed socket must not kill the server
    signal(SIGPIPE, SIG_IGN);
}



bool    Server::start( void )
{
    if (!(bind_socket()))
        return false;
    if (!(server_listen()))
        return false;
    installSignalHandlers();
    wait_poll();
    return true;
}

Server::~Server() 
{
    while (!(_clients.empty()))
    {
        int fd = _clients.begin()->first;
        close(fd);
        removeClient(fd);
    }
    for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        delete it->second;
    }
    _channels.clear();
    if (_server_fd != -1)
        close(_server_fd);
}

const std::string& Server::getPwd() const
{
    return _password;
}

Client* Server::findClientFd(int fd)
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return NULL;
    return it->second;
}

Client* Server::findClientNickname(std::string name)
{
    std::map<std::string, Client*>::iterator it = _clients_byNickname.find(name);
    if (it == _clients_byNickname.end())
        return NULL;
    return it->second;
}

Channel* Server::findChannel(std::string name)
{
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it == _channels.end())
        return NULL;
    return it->second;
}

void Server::removeChannel(Channel* channel)
{
    if (channel == NULL)
        return ;
    _channels.erase(channel->getName());
    delete channel;
}

void Server::setClientNickname(Client* client, const std::string& nickname)
{
    if (client->hasNickname())
        _clients_byNickname.erase(client->getNickname());
    client->setNickname(nickname);
    _clients_byNickname[nickname] = client;
}