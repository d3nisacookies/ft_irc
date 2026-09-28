#pragma once

#include "Server.hpp"
#include "Client.hpp"
#include "IRCMessage.hpp"

#include <vector>
#include <string>
#include <map>
#include <sstream>

struct Response
{
    Client* destination;
    std::string message;
};

enum Modes
{
    MODE_I_ON,
    MODE_I_OFF,
    MODE_T_ON,
    MODE_T_OFF,
    MODE_K_ON,
    MODE_K_OFF,
    MODE_O_ON,
    MODE_O_OFF,
    MODE_L_ON,
    MODE_L_OFF,
    INVALID_MODE
};

class CommandHandler
{
    private:
        Server* _server;

        void addResponse(Client* client, const std::string& message, std::vector<Response>& response);
        void addChannelResponse(Channel* channel, const std::string& message, std::vector<Response>& response);
        std::string getClientPrefix(Client* client);
        Modes resolveModes(const std::string& input);

    public:
        CommandHandler(Server* server);
        ~CommandHandler();

        std::vector<Response> processCommand(Client* client, const IRCMessage* irc_msg);

        void passCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response);
        void nickCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response);
        void userCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response);
        void joinCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response);
        void partCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response);
        void privmsgCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response);
        void kickCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response);
        void inviteCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response);
        void topicCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response);
        void modeCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response);
};
