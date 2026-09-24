#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

Server::Server(int port, const std::string& password) 
    : _port(port), _server_fd(-1), _password(password)
{
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
        return -1;
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
        return 0;
    }
    return 1;
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

void    Server::ValidateNewClient( void )
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(_server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
/* Authenticate here
    send(client_fd, WELCOME, sizeof(WELCOME), 0);  
    char buffer[128];
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (buffer)
*/
    send(client_fd, WELCOME, sizeof(WELCOME), 0);
    if (client_fd != -1)
    {
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        const char *host = inet_ntoa(client_addr.sin_addr);
        Client *client = new Client(client_fd, host);
	    client->setNickname("Random" + intToString(std::rand() % 101));
        _clients[client_fd] = client;

        std::cout << "Client named " << _clients[client_fd]->getNickname()
         << " connected from ip : " << host << std::endl;
        genNewPollfd( client_fd );
    }
    
}

void    Server::handleClient( const int client_fd , size_t &index)
{
    char buffer[1024];
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0)
    {
        buffer[n] = '\0';
        std::cout << buffer << std::endl;
        ++index;
    }
    else if (n == 0)
    {
        std::cout << _clients[client_fd]->getNickname() << " just disconnected." << std::endl;
        delete _clients[client_fd];
        _clients.erase(client_fd);
        close(client_fd);
        _all_fds.erase(_all_fds.begin() + index);
    }
    else
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            ++index;
        else
        {
            std::cout << "Client error: " << client_fd << std::endl;
            delete _clients[client_fd];
            _clients.erase(client_fd);
            close(client_fd);
            _all_fds.erase(_all_fds.begin() + index);
        }
    }
}

void    Server::wait_poll( void )
{
    while (true)
    {
        int ret = poll(_all_fds.data(), _all_fds.size(), -1);
        if (ret == -1)
        {
            perror("poll");
            break;
        }
        for (size_t i = 0; i < _all_fds.size();)
        {
            if (_all_fds[i].revents & POLLIN)
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


bool    Server::start( void )
{
    if (!(bind_socket()))
        return false;
    if (!(server_listen()))
        return false;
    wait_poll();
    return true;
}

Server::~Server() {}