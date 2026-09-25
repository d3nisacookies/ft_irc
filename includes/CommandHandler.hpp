#pragma once

#include "Server.hpp"
#include "Client.hpp"
#include "IRCMsgParser.hpp"

#include <vector>
#include <string>

struct Response
{
    Client* destination;
    std::string message;
};

class CommandHandler
{
    private:
        Server* _server;

    public:
        CommandHandler(Server* server);
        ~CommandHandler();

        std::vector<Response> processCommand(Client* client, const IRCMsgParser* irc_msg);
        void passCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response);
        void nickCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response);
        void userCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response);
        void joinCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response);
        void partCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response);
        void privmsgCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response);
        void kickCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response);
        void inviteCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response);
        void topicCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response);
        void modeCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response);
        
        
        /*
    Reference/pointer to Server
    Command dispatch function
    PASS 
    NICK 
    USER 
    JOIN 
    PART 
    PRIVMSG 
    KICK 
    INVITE 
    TOPIC 
    MODE 
    Helper functions for common validation
    Helper functions for creating/sending IRC responses ??
    */
};