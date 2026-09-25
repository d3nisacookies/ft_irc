#include "../includes/IRCMessage.hpp"

IRCMessage::IRCMessage(const std::string& raw)
{
    parse(raw);
}

void IRCMessage::parse(const std::string& raw)
{
    std::size_t pos = 0;
    std::size_t start;

    while(pos < raw.size() && raw[pos] == ' ')
        ++pos;
    start = pos;
    while (pos < raw.size() && raw[pos] != ' ')
        ++pos;
    _command = raw.substr(start, pos - start);

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