#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>


class Channel;

class Client
{
    private:
        int                         _fd;
        std::string                 _host;
        std::string                 _recvBuffer;

        bool                        _passVerified;
        bool                        _hasNickname;
        bool                        _hasUsername;
        
        std::string                 _nickname;
        std::string                 _username;
 
        std::vector<Channel *>      _channels;

        int                         _passAttempts;

    public:
        Client(int fd, const std::string& host);
        ~Client();

        int getFd() const;
        const std::string& getHost() const;
        const std::string& getRecvBuffer()const;

        bool isRegistered() const;
        bool isPassVerified() const;
        bool hasNickname() const;
        bool hasUsername() const;

        const std::string& getNickname() const;
        const std::string& getUsername() const;

        const std::vector<Channel *>& getChannels() const;

        int getPassAttempts() const;

        void appendRecvBuffer(const std::string& data);
        void clearBuffer();

        void setPassVerified(bool verified);
        void setNickname(const std::string& name);
        void setUsername(const std::string& name);

        void addChannel(Channel *channel);
        void removeChannel(Channel *channel);

        void increaseAttempt();
        void resetAttempt();
};



#endif