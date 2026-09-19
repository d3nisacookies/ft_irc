
#include "../includes/Client.hpp"
#include <iostream>

int main()
{
    Client c(4, "127.0.0.1");

    std::cout << "Initial registered: "
              << c.isRegistered() << std::endl;

    c.setNickname("testnick");
    std::cout << "After nickname, registered: "
              << c.isRegistered() << std::endl;

    c.setUsername("testuser");
    std::cout << "After username, registered: "
              << c.isRegistered() << std::endl;

    c.setPassVerified(true);
    std::cout << "After password verified, registered: "
              << c.isRegistered() << std::endl;

    c.increaseAttempt();
    c.increaseAttempt();
    c.increaseAttempt();

    std::cout << "Password attempts: "
              << c.getPassAttempts() << std::endl;

    c.resetAttempt();

    std::cout << "After reset: "
              << c.getPassAttempts() << std::endl;

    return 0;
}

