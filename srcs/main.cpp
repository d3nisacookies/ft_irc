#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <Server.hpp>

int main()
{
    Server *new_server = new Server(8080, "hello");
    std::cout << new_server << std::endl;
    if (!(new_server->start()))
        std::cerr << "Server start failed" << std::endl;
}