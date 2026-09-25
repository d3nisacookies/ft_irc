#pragma once

#include "Server.hpp"
#include "Client.hpp"
#include "IRCMessage.hpp"

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

        std::vector<Response> processCommand(Client* client, const IRCMessage* irc_msg);
        void passCmd(Client* client, const IRCMessage* irc_msg);
        void nickCmd(Client* client, const IRCMessage* irc_msg);
        void userCmd(Client* client, const IRCMessage* irc_msg);
        void joinCmd(Client* client, const IRCMessage* irc_msg);
        void partCmd(Client* client, const IRCMessage* irc_msg);
        void privmsgCmd(Client* client, const IRCMessage* irc_msg);
        void kickCmd(Client* client, const IRCMessage* irc_msg);
        void inviteCmd(Client* client, const IRCMessage* irc_msg);
        void topicCmd(Client* client, const IRCMessage* irc_msg);
        void modeCmd(Client* client, const IRCMessage* irc_msg);
        
        
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