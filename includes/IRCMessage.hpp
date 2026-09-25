#pragma once
#include <string>
#include <vector>


class IRCMessage
{
        private:
                std::string _command; // PRIVMSG
                std::vector<std::string> _params;

        public:
                IRCMessage( );
                IRCMessage(const std::string& raw);
                void parse(const std::string& raw);
                const std::string& getCommand() const;
                const std::vector<std::string>& getParams()const ;

};