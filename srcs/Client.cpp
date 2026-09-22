#include "../includes/Client.hpp"
#include <algorithm>

Client::Client(int fd, const std::string& host)
    : _fd(fd),
      _host(host),
      _recvBuffer(""),
      _passVerified(false),
      _hasNickname(false),
      _hasUsername(false),
      _nickname(""),
      _username(""),
      _channels(),
      _passAttempts(0)
{
}

Client::~Client(){}

int Client::getFd() const
{
    return _fd;
}

const std::string& Client::getHost() const
{
    return _host;
}

const std::string& Client::getRecvBuffer() const
{
    return _recvBuffer;
}

bool Client::isRegistered() const
{
    return _passVerified && _hasNickname && _hasUsername;
}

bool Client::isPassVerified() const
{
    return _passVerified;
}

bool Client::hasNickname() const
{
    return _hasNickname;
}

bool Client::hasUsername() const
{
    return _hasUsername;
}

const std::string& Client::getNickname() const
{
    return _nickname;
}

const std::string& Client::getUsername() const
{
    return _username;
}

const std::vector<Channel *>& Client::getChannels() const
{
    return _channels;
}

int Client::getPassAttempts() const
{
    return _passAttempts;
}

void Client::appendRecvBuffer(const std::string& data)
{
    _recvBuffer += data;
}

void Client::clearBuffer()
{
    _recvBuffer.clear();
}

void Client::setPassVerified(bool verified)
{
    _passVerified = verified;
}

void Client::setNickname(const std::string& name)
{
    _nickname = name;
    _hasNickname = true;
}

void Client::setUsername(const std::string& name)
{
    _username = name;
    _hasUsername = true;
}

void Client::addChannel(Channel *channel)
{
    if (channel == NULL)
        return;
    
    if (std::find(_channels.begin(), _channels.end(), channel) == _channels.end())
    {
        _channels.push_back(channel);
    }
}

void Client::removeChannel(Channel *channel)
{
    std::vector<Channel *>::iterator it;

    it = std::find(_channels.begin(), _channels.end(), channel);
    if (it != _channels.end())
        _channels.erase(it);
}

void Client::increaseAttempt()
{
    ++_passAttempts;
}

void Client::resetAttempt()
{
    _passAttempts = 0;
}

