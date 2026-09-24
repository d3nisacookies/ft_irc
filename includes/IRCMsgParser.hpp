#pragma once

// IRCMsgParser received raw msg then parse
// based on the command call command handler
#include <string>

class IRCMsgParser
{
    private:
                // "PRIVMSG #42 :Hello Bob!"
        std::string _command; // PRIVMSG
        std::string _parameter; // #42
        std::string _trailing;  // Hello Bob!
};