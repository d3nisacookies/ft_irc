#include "CommandHandler.hpp"

CommandHandler::CommandHandler(Server* server) : _server(server) {}

CommandHandler::~CommandHandler() {}

std::vector<Response> CommandHandler::processCommand(Client* client, const IRCMessage* irc_msg) 
{
    std::vector<Response> response_msg;
    
    std::string commands[10] = {"PASS", "NICK", "USER", "JOIN", "PART", 
        "PRIVMSG", "KICK", "INVITE", "TOPIC", "MODE"};

    void(CommandHandler::*functions[10])
        (Client*, const IRCMessage*, std::vector<Response>&) = {
        &CommandHandler::passCmd,
        &CommandHandler::nickCmd,
        &CommandHandler::userCmd,
        &CommandHandler::joinCmd,
        &CommandHandler::partCmd,
        &CommandHandler::privmsgCmd,
        &CommandHandler::kickCmd,
        &CommandHandler::inviteCmd,
        &CommandHandler::topicCmd,
        &CommandHandler::modeCmd,
    };

    for(int i = 0; i < 10; i++)
    {
        if(commands[i] == irc_msg->getCommand())
        {
            (this->*functions[i])(client, irc_msg, response_msg);
            return response_msg;
        }
    }
    return response_msg;
}

void CommandHandler::passCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response) 
{  
    // PASS empty? PASS abd abc?
    const std::vector<std::string>& parameters = irc_msg->getParams(); // need to valid the args of getParams
    if (parameters.size() != 1 || parameters[0].empty())
    {
        addResponse(client, "Pass follow this format: PASS password", response);
        return ; 
    }
    const std::string& enteredPwd = parameters[0];
    const std::string& pwd = _server->getPwd();

    if (enteredPwd == pwd)
    {
        client->setPassVerified(true);
        addResponse(client, "Password Correct!", response);
    } else
    {
        addResponse(client, "Wrong Password", response);
    }
}

void CommandHandler::nickCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response) 
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if (parameters.size() != 1)
    {
        addResponse(client, "NICK requires exactly one parameter: NICK nickname", response);
        return;
    }

    const std::string& nickname = parameters[0];

    if (!_server->nicknameExists(nickname))
    {
        _server->setClientNickname(client, nickname);
        addResponse(client, "Successfully set Nickname!", response);
    }
    else
    {
        addResponse(client, "Nickname already existed", response);
    }
}

void CommandHandler::userCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response) 
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if (parameters.size() != 4)
    {
        addResponse(client,  "USER requires 4 parameter: USER username hostname servername :realname ", response);
        return;
    }

    if (client->hasUsername())
    {
        addResponse(client, "Username has already been set", response);
        return;
    }
    const std::string& username = parameters[0];
    client->setUsername(username);
    addResponse(client, "Successfully set username", response);
}

void CommandHandler::joinCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response) 
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if(parameters.size() != 1 || parameters[0].empty() || parameters[0][0] != '#')
    {
        addResponse(client,  "JOIN requires 1 parameter in this format: JOIN #channel", response);
        return;
    }
    const std::string& channel_name = parameters[0];

    Channel* channel = _server->findChannel(channel_name);
    if(channel != NULL) // exist channel
    {
        // Channel exit: check whether the client is already inside the channel
        if(channel->isMember(client))
        {
            addResponse(client, "You are already in " + channel_name, response);
            return ;
        }
        channel->addMember(client);
        client->addChannel(channel);

        // notify join succeeded
        addResponse(client, "Join " + channel_name, response);
    } else
    { // new channel
        Channel *newChannel = new Channel(channel_name);
        _server->addChannel(channel_name,newChannel);
        newChannel->addMember(client);
        client->addChannel(newChannel);
        newChannel->addOp(client);
        addResponse(client, "Join " + channel_name, response);
    }
}

void CommandHandler::partCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response) 
{
}

void CommandHandler::privmsgCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response) 
{
}

void CommandHandler::kickCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response) 
{
}

void CommandHandler::inviteCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response) 
{
}

void CommandHandler::topicCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response) 
{
}

void CommandHandler::modeCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response) 
{
}

void CommandHandler::addResponse(Client* client, const std::string& message, std::vector<Response>& response)
{
    Response r;
    r.destination = client;
    r.message = message;
    response.push_back(r);
}
