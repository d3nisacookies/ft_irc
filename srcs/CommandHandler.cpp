#include "CommandHandler.hpp"

CommandHandler::CommandHandler(Server* server) : _server(server) {}

CommandHandler::~CommandHandler() {}

std::vector<Response> CommandHandler::processCommand(Client* client, const IRCMsgParser* irc_msg) {
    
    std::vector<Response> response_msg;
    
    std::string commands[10] = {"PASS", "NICK", "USER", "JOIN", "PART", 
        "PRIVMSG", "KICK", "INVITE", "TOPIC", "MODE"};

    void(CommandHandler::*functions[10])
        (Client*, const IRCMsgParser*, std::vector<Response>&) = {
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

void CommandHandler::passCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response) {
    // PASS empty? PASS abd abc?
    Response r;
    const std::string enteredPwd = irc_msg->getParameters();
    const std::string pwd = _server->getPwd();

    if (enteredPwd == pwd)
    {
        client->setPassVerified(true);
    } else
    {
        client->increaseAttempt();
        r.destination = client;
        r.message = "Wrong Password";
        response.push_back(r);
    }
}

void CommandHandler::nickCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response) {

    const std::string nickname = irc_msg->getParameters();
    Response r;
    if (_server->findClientNickname(nickname) == NULL)
    {
        client->setNickname(nickname);
    } else
    {
        r.destination = client;
        r.message = "Nickname already existed";
        response.push_back(r);
    }
}

void CommandHandler::userCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response) {

}
void CommandHandler::joinCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response) {

}
void CommandHandler::partCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response) {

}
void CommandHandler::privmsgCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response) {

}
void CommandHandler::kickCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response) {

}
void CommandHandler::inviteCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response) {

}
void CommandHandler::topicCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response) {

}
void CommandHandler::modeCmd(Client* client, const IRCMsgParser* irc_msg, std::vector<Response>& response) {

}