#include "CommandHandler.hpp"

#include <cctype>
#include <climits>

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
        &CommandHandler::modeCmd
    };

    for (int i = 0; i < 10; i++)
    {
        if (commands[i] == irc_msg->getCommand())
        {
            (this->*functions[i])(client, irc_msg, response_msg);
            return response_msg;
        }
    }
    return response_msg;
}

std::string CommandHandler::getClientPrefix(Client* client)
{
    std::string prefix = ":";

    if (!client->getNickname().empty())
        prefix += client->getNickname();
    else
        prefix += "*";

    if (!client->getUsername().empty())
    {
        prefix += "!";
        prefix += client->getUsername();
        prefix += "@";
        prefix += client->getHost();
    }
    return prefix;
}

void CommandHandler::addChannelResponse(Channel* channel, const std::string& message, std::vector<Response>& response)
{
    const std::set<Client*>& members = channel->getMembers();

    for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
        addResponse(*it, message, response);
}

void CommandHandler::passCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response)
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if (parameters.size() != 1 || parameters[0].empty())
    {
        addResponse(client, ":ircserv 461 " + client->getNickname() + " PASS :Not enough parameters\r\n", response);
        return;
    }

    const std::string& enteredPwd = parameters[0];
    const std::string& pwd = _server->getPwd();

    if (enteredPwd == pwd)
    {
        client->setPassVerified(true);
        addResponse(client, ":ircserv 202 " + client->getNickname() + " :Password accepted\r\n", response);
    }
    else
    {
        addResponse(client, ":ircserv 464 " + client->getNickname() + " :Password incorrect\r\n", response);
    }
}

void CommandHandler::nickCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response)
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if (parameters.size() != 1 || parameters[0].empty())
    {
        addResponse(client, ":ircserv 431 " + client->getNickname() + " :No nickname given\r\n", response);
        return;
    }

    const std::string& nickname = parameters[0];

    /*
    IRC nickname validation.
    First character: letter or [ ] \ ` _ ^ { | }
    Following characters: letter, number or special character, + '-'
     */
    if (!std::isalpha(static_cast<unsigned char>(nickname[0])) &&
        nickname[0] != '[' && nickname[0] != ']' &&
        nickname[0] != '\\' && nickname[0] != '`' &&
        nickname[0] != '_' && nickname[0] != '^' &&
        nickname[0] != '{' && nickname[0] != '|' &&
        nickname[0] != '}')
    {
        addResponse(client, ":ircserv 432 " + client->getNickname() + " " +
            nickname + " :Erroneous nickname\r\n", response);
        return;
    }

    if (nickname.size() > 9)
    {
        addResponse(client, ":ircserv 432 " + client->getNickname() + " " +
            nickname + " :Erroneous nickname\r\n", response);
        return;
    }

    for (size_t i = 1; i < nickname.size(); i++)
    {
        if (!std::isalnum(static_cast<unsigned char>(nickname[i])) &&
            nickname[i] != '[' && nickname[i] != ']' &&
            nickname[i] != '\\' && nickname[i] != '`' &&
            nickname[i] != '_' && nickname[i] != '^' &&
            nickname[i] != '{' && nickname[i] != '|' &&
            nickname[i] != '}' && nickname[i] != '-')
        {
            addResponse(client, ":ircserv 432 " + client->getNickname() + " " +
                nickname + " :Erroneous nickname\r\n", response);
            return;
        }
    }

    if (_server->nicknameExist(nickname) &&
        nickname != client->getNickname())
    {
        addResponse(client, ":ircserv 433 " + client->getNickname() + " " +
            nickname + " :Nickname is already in use\r\n", response);
        return;
    }

    std::string oldPrefix = getClientPrefix(client);

    
    //If the client aldy has nickname, everyone in the channels should get NICK message.
    //Save the channels before changing anything.
    
    std::vector<Channel*> channels;

    const std::vector<Channel*>& clientChannels = client->getChannels();

    for (std::vector<Channel*>::const_iterator it = clientChannels.begin(); it != clientChannels.end(); ++it)
        channels.push_back(*it);

    _server->setClientNickname(client, nickname);

    std::string message = oldPrefix + " NICK :" + nickname + "\r\n";

    if (channels.empty())
        addResponse(client, message, response);
    else
    {
        for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it)
        {
            const std::set<Client*>& members = (*it)->getMembers();
            for (std::set<Client*>::const_iterator member = members.begin(); member != members.end(); ++member)
                addResponse(*member, message, response);
        }
    }
}

void CommandHandler::userCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response)
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if (parameters.size() != 4)
    {
        addResponse(client, ":ircserv 461 " + client->getNickname() +
            " USER :Not enough parameters\r\n", response);
        return;
    }

    if (client->hasUsername())
    {
        addResponse(client, ":ircserv 462 " + client->getNickname() +
            " :You may not reregister\r\n", response);
        return;
    }

    if (parameters[0].empty())
    {
        addResponse(client, ":ircserv 461 " + client->getNickname() +
            " USER :Not enough parameters\r\n", response);
        return;
    }

    const std::string& username = parameters[0];
    client->setUsername(username);
    addResponse(client, ":ircserv 202 " + client->getNickname() +
        " :Username accepted\r\n", response);
}

void CommandHandler::joinCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response)
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if (parameters.size() < 1 || parameters.size() > 2 ||
        parameters[0].empty() || parameters[0][0] != '#')
    {
        addResponse(client, ":ircserv 461 " + client->getNickname() +
            " JOIN :Not enough parameters\r\n", response);
        return;
    }

    const std::string& channel_name = parameters[0];
    Channel* channel = _server->findChannel(channel_name);

    if (channel != NULL)
    {
        if (channel->isMember(client))
        {
            addResponse(client, ":ircserv 443 " + client->getNickname() + " " +
                client->getNickname() + " " + channel_name +
                " :is already on channel\r\n", response);
            return;
        }

        if (channel->isKey())
        {
            if (parameters.size() != 2 ||
                parameters[1] != channel->getKey())
            {
                addResponse(client, ":ircserv 475 " + client->getNickname() + " " +
                    channel_name + " :Cannot join channel (+k)\r\n",
                    response);
                return;
            }
        }

        if (channel->isUserLimitEnable() &&
            channel->getMembers().size() >=
            static_cast<size_t>(channel->getLimit()))
        {
            addResponse(client, ":ircserv 471 " + client->getNickname() + " " +
                channel_name + " :Cannot join channel (+l)\r\n",
                response);
            return;
        }

        if (channel->isInviteOnly() && !channel->isInvited(client))
        {
            addResponse(client, ":ircserv 473 " + client->getNickname() + " " +
                channel_name + " :Cannot join channel (+i)\r\n",
                response);
            return;
        }

        channel->addMember(client);
        client->addChannel(channel);

        if (channel->isInvited(client))
            channel->removeInvitedClient(client);

        std::string message = getClientPrefix(client) +
            " JOIN :" + channel_name + "\r\n";

        addChannelResponse(channel, message, response);
    }
    else
    {
        Channel* newChannel = new Channel(channel_name);

        _server->addChannel(channel_name, newChannel);

        newChannel->addMember(client);
        client->addChannel(newChannel);
        newChannel->addOp(client);

        std::string message = getClientPrefix(client) +
            " JOIN :" + channel_name + "\r\n";

        addResponse(client, message, response);
    }
}

void CommandHandler::partCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response)
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if (parameters.size() < 1 || parameters.size() > 2 ||
        parameters[0].empty() || parameters[0][0] != '#')
    {
        addResponse(client, ":ircserv 461 " + client->getNickname() +
            " PART :Not enough parameters\r\n", response);
        return;
    }

    const std::string& channel_name = parameters[0];
    std::string reason;

    if (parameters.size() == 2)
        reason = parameters[1];

    Channel* channel = _server->findChannel(channel_name);

    if (channel == NULL)
    {
        addResponse(client, ":ircserv 403 " + client->getNickname() + " " +
            channel_name + " :No such channel\r\n", response);
        return;
    }

    if (!channel->isMember(client))
    {
        addResponse(client, ":ircserv 442 " + client->getNickname() + " " +
            channel_name + " :You're not on that channel\r\n",
            response);
        return;
    }

    std::string message = getClientPrefix(client) +
        " PART " + channel_name;

    if (!reason.empty())
        message += " :" + reason;

    message += "\r\n";


    //Broadcast before removing client so the client itself
    //and other members all receive the PART.
    addChannelResponse(channel, message, response);

    channel->removeMember(client);
    client->removeChannel(channel);

    //Server owns the Channel, so let Server remove/delete it.
    if (channel->getMembers().empty())
        _server->removeChannel(channel);
}

void CommandHandler::privmsgCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response)
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if (parameters.size() != 2 ||
        parameters[0].empty() || parameters[1].empty())
    {
        addResponse(client, ":ircserv 461 " + client->getNickname() +
            " PRIVMSG :Not enough parameters\r\n", response);
        return;
    }
    const std::string& target = parameters[0];
    const std::string& msg = parameters[1];
    std::string message = getClientPrefix(client) + " PRIVMSG " + target + " :" + msg + "\r\n";

    // PRIVMSG to a channel.
    if (target[0] == '#')
    {
        Channel* channel = _server->findChannel(target);

        if (channel == NULL)
        {
            addResponse(client, ":ircserv 403 " + client->getNickname() + " " +
                target + " :No such channel\r\n", response);
            return;
        }

        if (!channel->isMember(client))
        {
            addResponse(client, ":ircserv 442 " + client->getNickname() + " " +
                target + " :You're not on that channel\r\n", response);
            return;
        }

        const std::set<Client*>& members = channel->getMembers();

        for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
        {
            //Do not send the PRIVMSG back to the sender.
            if (*it != client)
                addResponse(*it, message, response);
        }

        return;
    }

    // PRIVMSG to another user.
    Client* targetClient = _server->findClientNickname(target);

    if (targetClient == NULL)
    {
        addResponse(client, ":ircserv 401 " + client->getNickname() + " " +
            target + " :No such nick/channel\r\n", response);
        return;
    }
    addResponse(targetClient, message, response);
}

void CommandHandler::kickCmd(Client* client,
    const IRCMessage* irc_msg, std::vector<Response>& response)
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if (parameters.size() < 2 || parameters.size() > 3 ||
        parameters[0].empty() || parameters[0][0] != '#' ||
        parameters[1].empty())
    {
        addResponse(client, ":ircserv 461 " + client->getNickname() +
            " KICK :Not enough parameters\r\n", response);
        return;
    }

    const std::string& channelName = parameters[0];
    const std::string& targetNickname = parameters[1];

    std::string reason;

    if (parameters.size() == 3)
        reason = parameters[2];

    Channel* channel = _server->findChannel(channelName);

    if (channel == NULL)
    {
        addResponse(client, ":ircserv 403 " + client->getNickname() + " " +
            channelName + " :No such channel\r\n", response);
        return;
    }

    if (!channel->isMember(client))
    {
        addResponse(client, ":ircserv 442 " + client->getNickname() + " " +
            channelName + " :You're not on that channel\r\n", response);
        return;
    }

    if (!channel->isOp(client))
    {
        addResponse(client, ":ircserv 482 " + client->getNickname() + " " +
            channelName + " :You're not channel operator\r\n", response);
        return;
    }

    Client* targetClient = _server->findClientNickname(targetNickname);

    if (targetClient == NULL)
    {
        addResponse(client, ":ircserv 401 " + client->getNickname() + " " +
            targetNickname + " :No such nick/channel\r\n", response);
        return;
    }
    if (!channel->isMember(targetClient))
    {
        addResponse(client, ":ircserv 441 " + client->getNickname() + " " + targetNickname + " " + channelName +
            " :They aren't on that channel\r\n", response);
        return;
    }

    std::string message = getClientPrefix(client) + " KICK " + channelName + " " + targetNickname;

    if (!reason.empty())
        message += " :" + reason;
    else
        message += " :" + client->getNickname();

    message += "\r\n";

    //Broadcast before removing target.
    addChannelResponse(channel, message, response);

    channel->removeMember(targetClient);
    targetClient->removeChannel(channel);

    if (channel->getMembers().empty())
        _server->removeChannel(channel);
}

void CommandHandler::inviteCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response)
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if (parameters.size() != 2 || parameters[0].empty() || 
        parameters[1].empty() || parameters[1][0] != '#')
    {
        addResponse(client, ":ircserv 461 " + client->getNickname() +
            " INVITE :Not enough parameters\r\n", response);
        return;
    }

    const std::string& targetNickname = parameters[0];
    const std::string& channelName = parameters[1];

    Channel* channel = _server->findChannel(channelName);

    if (channel == NULL)
    {
        addResponse(client, ":ircserv 403 " + client->getNickname() + " " +
            channelName + " :No such channel\r\n", response);
        return;
    }

    if (!channel->isMember(client))
    {
        addResponse(client, ":ircserv 442 " + client->getNickname() + " " +
            channelName + " :You're not on that channel\r\n",
            response);
        return;
    }

    // For +i, only operators can invite.
    if (!channel->isOp(client))
    {
        addResponse(client, ":ircserv 482 " + client->getNickname() + " " +
            channelName + " :You're not channel operator\r\n", response);
        return;
    }

    Client* targetClient = _server->findClientNickname(targetNickname);

    if (targetClient == NULL)
    {
        addResponse(client, ":ircserv 401 " + client->getNickname() + " " +
            targetNickname + " :No such nick/channel\r\n", response);
        return;
    }

    if (channel->isMember(targetClient))
    {
        addResponse(client, ":ircserv 443 " + client->getNickname() + " " +
            targetNickname + " " + channelName + " :is already on channel\r\n", response);
        return;
    }

    channel->addInvitedClient(targetClient);
    
    //The invited user receives INVITE msg.
    std::string inviteMessage = getClientPrefix(client) +
        " INVITE " + targetNickname + " :" + channelName + "\r\n";

    addResponse(targetClient, inviteMessage, response);

    // Inviter also receives confirmation.
    addResponse(client, ":ircserv 341 " + client->getNickname() + " " +
        targetNickname + " " + channelName + "\r\n", response);
}

void CommandHandler::topicCmd(Client* client, const IRCMessage* irc_msg, std::vector<Response>& response)
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    if (parameters.size() < 1 || parameters.size() > 2 ||
        parameters[0].empty() || parameters[0][0] != '#')
    {
        addResponse(client, ":ircserv 461 " + client->getNickname() +
            " TOPIC :Not enough parameters\r\n", response);
        return;
    }

    const std::string& channel_name = parameters[0];

    Channel* channel = _server->findChannel(channel_name);

    if (channel == NULL)
    {
        addResponse(client, ":ircserv 403 " + client->getNickname() + " " +
            channel_name + " :No such channel\r\n", response);
        return;
    }

    if (!channel->isMember(client))
    {
        addResponse(client, ":ircserv 442 " + client->getNickname() + " " +
            channel_name + " :You're not on that channel\r\n", response);
        return;
    }

    // if TOPIC #channel return current topic
    if (parameters.size() == 1)
    {
        if (channel->getTopic().empty())
        {
            addResponse(client, ":ircserv 331 " + client->getNickname() + " " +
                channel_name + " :No topic is set\r\n", response);
        }
        else
        {
            addResponse(client, ":ircserv 332 " + client->getNickname() + " " +
                channel_name + " :" + channel->getTopic() + "\r\n", response);
        }
        return;
    }

    // TOPIC #channel :new topic
    if (channel->isTopicRestricted() && !channel->isOp(client))
    {
        addResponse(client, ":ircserv 482 " + client->getNickname() + " " +
            channel_name + " :You're not channel operator\r\n", response);
        return;
    }
    // if ops change topic
    channel->setTopic(parameters[1]);

    std::string message = getClientPrefix(client) +
        " TOPIC " + channel_name + " :" + parameters[1] + "\r\n";

    addChannelResponse(channel, message, response);
}

Modes CommandHandler::resolveModes(const std::string& input)
{
    if (input == "+i")
        return MODE_I_ON;
    if (input == "-i")
        return MODE_I_OFF;
    if (input == "+t")
        return MODE_T_ON;
    if (input == "-t")
        return MODE_T_OFF;
    if (input == "+k")
        return MODE_K_ON;
    if (input == "-k")
        return MODE_K_OFF;
    if (input == "+o")
        return MODE_O_ON;
    if (input == "-o")
        return MODE_O_OFF;
    if (input == "+l")
        return MODE_L_ON;
    if (input == "-l")
        return MODE_L_OFF;
    return INVALID_MODE;
}

void CommandHandler::modeCmd(Client* client,
    const IRCMessage* irc_msg, std::vector<Response>& response)
{
    const std::vector<std::string>& parameters = irc_msg->getParams();

    // MODE #channel return current active modes.
    if (parameters.size() == 1 &&
        !parameters[0].empty() && parameters[0][0] == '#')
    {
        Channel* channel = _server->findChannel(parameters[0]);

        if (channel == NULL)
        {
            addResponse(client, ":ircserv 403 " + client->getNickname() + " " +
                parameters[0] + " :No such channel\r\n", response);
            return;
        }

        if (!channel->isMember(client))
        {
            addResponse(client, ":ircserv 442 " + client->getNickname() + " " +
                parameters[0] + " :You're not on that channel\r\n", response);
            return;
        }

        std::string modes = "+";

        if (channel->isInviteOnly())
            modes += "i";

        if (channel->isTopicRestricted())
            modes += "t";

        if (channel->isKey())
            modes += "k";

        if (channel->isUserLimitEnable())
            modes += "l";

        addResponse(client, ":ircserv 324 " + client->getNickname() + " " +
            parameters[0] + " " + modes + "\r\n", response);

        return;
    }

    if (parameters.size() < 2 || parameters.size() > 3 || parameters[0].empty() || 
        parameters[0][0] != '#' || parameters[1].empty())
    {
        addResponse(client, ":ircserv 461 " + client->getNickname() +
            " MODE :Not enough parameters\r\n", response);
        return;
    }

    const std::string& channel_name = parameters[0];
    const std::string& mode = parameters[1];

    Channel* channel = _server->findChannel(channel_name);

    if (channel == NULL)
    {
        addResponse(client, ":ircserv 403 " + client->getNickname() + " " +
            channel_name + " :No such channel\r\n", response);
        return;
    }

    if (!channel->isMember(client))
    {
        addResponse(client, ":ircserv 442 " + client->getNickname() + " " +
            channel_name + " :You're not on that channel\r\n", response);
        return;
    }

    if (!channel->isOp(client))
    {
        addResponse(client, ":ircserv 482 " + client->getNickname() + " " +
            channel_name + " :You're not channel operator\r\n", response);
        return;
    }

    Modes selectedMode = resolveModes(mode);
    switch (selectedMode)
    {
        case MODE_I_ON:
        {
            if (parameters.size() != 2)
            {
                addResponse(client, ":ircserv 461 " + client->getNickname() +
                    " MODE :MODE #channel +i\r\n", response);
                return;
            }
            channel->changeInviteStatus(true);
            break;
        }

        case MODE_I_OFF:
        {
            if (parameters.size() != 2)
            {
                addResponse(client, ":ircserv 461 " + client->getNickname() +
                    " MODE :MODE #channel -i\r\n", response);
                return;
            }
            channel->changeInviteStatus(false);
            break;
        }

        case MODE_T_ON:
        {
            if (parameters.size() != 2)
            {
                addResponse(client, ":ircserv 461 " + client->getNickname() +
                    " MODE :MODE #channel +t\r\n", response);
                return;
            }
            channel->changeTopicRestriction(true);
            break;
        }

        case MODE_T_OFF:
        {
            if (parameters.size() != 2)
            {
                addResponse(client, ":ircserv 461 " + client->getNickname() +
                    " MODE :MODE #channel -t\r\n", response);
                return;
            }
            channel->changeTopicRestriction(false);
            break;
        }

        case MODE_K_ON:
        {
            if (parameters.size() != 3 || parameters[2].empty())
            {
                addResponse(client, ":ircserv 461 " + client->getNickname() +
                    " MODE :MODE #channel +k password\r\n", response);
                return;
            }
            channel->changeKey(parameters[2]);
            break;
        }

        case MODE_K_OFF:
        {
            if (parameters.size() != 2)
            {
                addResponse(client, ":ircserv 461 " + client->getNickname() +
                    " MODE :MODE #channel -k\r\n", response);
                return;
            }
            channel->disableKey();
            break;
        }

        case MODE_O_ON:
        {
            if (parameters.size() != 3 || parameters[2].empty())
            {
                addResponse(client, ":ircserv 461 " + client->getNickname() +
                    " MODE :MODE #channel +o nickname\r\n", response);
                return;
            }
            const std::string& targetNickname = parameters[2];

            Client* targetClient =
                _server->findClientNickname(targetNickname);

            if (targetClient == NULL)
            {
                addResponse(client, ":ircserv 401 " + client->getNickname() + " " +
                    targetNickname + " :No such nick/channel\r\n", response);
                return;
            }

            if (!channel->isMember(targetClient))
            {
                addResponse(client, ":ircserv 441 " + client->getNickname() + " " +
                    targetNickname + " " + channel_name + " :They aren't on that channel\r\n", response);
                return;
            }

            if (channel->isOp(targetClient))
            {
                addResponse(client, ":ircserv 696 " + client->getNickname() + " " +
                    channel_name + " +o " + targetNickname + " :User is already an operator\r\n", response);
                return;
            }
            channel->addOp(targetClient);
            break;
        }

        case MODE_O_OFF:
        {
            if (parameters.size() != 3 || parameters[2].empty())
            {
                addResponse(client, ":ircserv 461 " + client->getNickname() +
                    " MODE :MODE #channel -o nickname\r\n", response);
                return;
            }

            const std::string& targetNickname = parameters[2];

            Client* targetClient =
                _server->findClientNickname(targetNickname);

            if (targetClient == NULL)
            {
                addResponse(client, ":ircserv 401 " + client->getNickname() + " " +
                    targetNickname + " :No such nick/channel\r\n", response);
                return;
            }

            if (!channel->isMember(targetClient))
            {
                addResponse(client, ":ircserv 441 " + client->getNickname() + " " +
                    targetNickname + " " + channel_name + " :They aren't on that channel\r\n", response);
                return;
            }

            if (!channel->isOp(targetClient))
            {
                addResponse(client, ":ircserv 696 " + client->getNickname() + " " +
                    channel_name + " -o " + targetNickname +
                    " :User is not an operator\r\n", response);
                return;
            }

            // Keep at least one operator
            if (targetClient == client &&
                channel->getOperators().size() == 1)
            {
                addResponse(client, ":ircserv 482 " + client->getNickname() + " " +
                    channel_name + " :Cannot remove the last operator\r\n", response);
                return;
            }

            channel->removeOp(targetClient);
            break;
        }

        case MODE_L_ON:
        {
            if (parameters.size() != 3 || parameters[2].empty())
            {
                addResponse(client, ":ircserv 461 " + client->getNickname() +
                    " MODE :MODE #channel +l limit\r\n", response);
                return;
            }

            std::istringstream ss(parameters[2]);
            int limit;

            if (!(ss >> limit) || !ss.eof() || limit <= 0 || limit > INT_MAX)
            {
                addResponse(client, ":ircserv 461 " + client->getNickname() +
                    " MODE :Invalid channel limit\r\n", response);
                return;
            }

            channel->changeLimit(limit);
            break;
        }

        case MODE_L_OFF:
        {
            if (parameters.size() != 2)
            {
                addResponse(client, ":ircserv 461 " + client->getNickname() +
                    " MODE :MODE #channel -l\r\n", response);
                return;
            }

            channel->disableLimit();
            break;
        }

        default:
        {
            addResponse(client, ":ircserv 472 " + client->getNickname() + " " +
                mode + " :is unknown mode char to me\r\n", response);
            return;
        }
    }

    // Broadcast the successful MODE change.
    std::string modeMessage = getClientPrefix(client) + " MODE " + channel_name + " " + mode;

    if (parameters.size() == 3)
        modeMessage += " " + parameters[2];

    modeMessage += "\r\n";

    addChannelResponse(channel, modeMessage, response);
}

void CommandHandler::addResponse(Client* client, const std::string& message, std::vector<Response>& response)
{
    Response r;
    r.destination = client;
    r.message = message;
    response.push_back(r);
}
