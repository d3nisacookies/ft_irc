#pragma once

// IRCMsgParser received raw msg then parse
// based on the command call command handler
#include <string>

class IRCMsgParser
{
    private:
        // "PRIVMSG #42 :Hello Bob!"
        // Pass mypassword
        std::string _command; // PRIVMSG // Pass
        std::string _parameter; // #42 // mypassword
        std::string _trailing;  // Hello Bob! // ""

    public:
        const std::string& getCommand() const;
        const std::string& getParameters() const;
        const std::string& getTrailing()const;
};