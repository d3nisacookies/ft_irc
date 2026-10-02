#include "../includes/IRCMessage.hpp"

IRCMessage::IRCMessage( )
{
}

IRCMessage::IRCMessage(const std::string& raw)
{
    parse(raw);
}


// Notes for AK
// parser need to reset the _params once done with the command
// need to handle empty command

bool IRCMessage::isEmpty() const
{
    if (_command.empty())
        return true;
    return false;
}

void    IRCMessage::parse(const std::string& raw)
{
    _params.clear();
    _command.clear();

    std::size_t pos = 0;
    std::size_t start;

    while(pos < raw.size() && raw[pos] == ' ')
        ++pos;
    if (pos < raw.size() && raw[pos] == ':')
    {
        while (pos < raw.size() && raw[pos] != ' ')
            ++pos;
        while (pos < raw.size() && raw[pos] == ' ')
            ++pos;
    }

    start = pos;
    while (pos < raw.size() && raw[pos] != ' ')
        ++pos;
    _command = raw.substr(start, pos - start);
    for (std::size_t i = 0; i < _command.size(); ++i)
    {
        _command[i] = static_cast<unsigned char>(std::toupper(_command[i]));
    }

    while (pos < raw.size())
    {
        while (pos < raw.size() && raw[pos] == ' ')
            ++pos;
        if (pos >= raw.size())
            break;
        if (raw[pos] == ':')
        {
            _params.push_back(raw.substr(pos + 1));
            break;
        }
        start = pos;
        while (pos < raw.size() && raw[pos] != ' ')
                ++pos;
        _params.push_back(raw.substr(start, pos - start));
    }
}

const std::string& IRCMessage::getCommand()const
{
    return _command;
}

const std::vector<std::string>& IRCMessage::getParams()const
{
    return _params;
}