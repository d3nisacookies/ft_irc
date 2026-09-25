#include "IRCMessage.hpp"
#include <iostream>
#include <vector>

int main()
{
    IRCMessage msg("PASS :secret");

    std::cout << "Command: " << msg.getCommand() << std::endl;

    const std::vector<std::string>& params = msg.getParams();

    for (size_t i = 0; i < params.size(); ++i)
        std::cout << "Param " << i << ": [" << params[i] << "]" << std::endl;

    return 0;
}