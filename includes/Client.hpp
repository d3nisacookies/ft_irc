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
        std::string                 _sendBuffer;

        bool                        _passVerified;
        bool                        _hasNickname;
        bool                        _hasUsername;
        bool                        _welcomed;
        bool                        _quitting;
        
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
        const std::string& getSendBuffer() const;
        void appendSendBuffer(const std::string& data);
        void consumeSendBuffer(size_t n);

        bool isRegistered() const;
        bool isPassVerified() const;
        bool hasNickname() const;
        bool hasUsername() const;
        bool extractLine(std::string& line);
        bool isWelcomed() const;
        void setWelcomed(bool welcomed);
        bool isQuitting() const;

        const std::string& getNickname() const;
        const std::string& getUsername() const;

        const std::vector<Channel *>& getChannels() const;

        int getPassAttempts() const;

        void appendRecvBuffer(const std::string& data);
        void clearBuffer();

        void setPassVerified(bool verified);
        void setNickname(const std::string& name);
        void setUsername(const std::string& name);
        void setQuitting(bool quitting);

        void addChannel(Channel *channel);
        void removeChannel(Channel *channel);

        void increaseAttempt();
        void resetAttempt();
};



#endif