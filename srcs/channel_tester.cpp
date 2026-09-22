#include "Channel.hpp"
#include "Client.hpp"

#include <iostream>

int main()
{
    std::cout << "===== CHANNEL TESTER =====" << std::endl;

    Client alice(1, "alice");
    Client bob(2, "bob");
    Client charlie(3, "charlie");

    Channel channel("#test");


    std::cout << "\n--- Basic Channel ---" << std::endl;

    std::cout << "Channel name: "
              << channel.getName() << std::endl;

    std::cout << "Topic: '"
              << channel.getTopic() << "'" << std::endl;


    std::cout << "\n--- Members ---" << std::endl;

    channel.addMember(&alice);
    channel.addMember(&bob);

    std::cout << "Alice member: "
              << channel.isMember(&alice) << std::endl;

    std::cout << "Bob member: "
              << channel.isMember(&bob) << std::endl;

    std::cout << "Charlie member: "
              << channel.isMember(&charlie) << std::endl;

    // Duplicate member
    channel.addMember(&alice);

    std::cout << "\n--- Operators ---" << std::endl;

    channel.addOp(&alice);

    std::cout << "Alice operator: "
              << channel.isOp(&alice) << std::endl;

    std::cout << "Bob operator: "
              << channel.isOp(&bob) << std::endl;

    // Try making non-member an operator
    channel.addOp(&charlie);

    // Remove operator status
    channel.removeOp(&alice);

    std::cout << "Alice operator after removal: "
              << channel.isOp(&alice) << std::endl;

    std::cout << "\n--- Topic ---" << std::endl;

    channel.setTopic("Welcome to the test channel!");

    std::cout << "Topic: "
              << channel.getTopic() << std::endl;


    std::cout << "\n--- +i Invite Only ---" << std::endl;

    std::cout << "Invite only: "
              << channel.isInviteOnly() << std::endl;

    channel.changeInviteStatus();

    std::cout << "Invite only after +i: "
              << channel.isInviteOnly() << std::endl;

    channel.changeInviteStatus();

    std::cout << "Invite only after -i: "
              << channel.isInviteOnly() << std::endl;

    std::cout << "\n--- Invitations ---" << std::endl;

    channel.addInvitedClient(&charlie);

    std::cout << "Charlie invited: "
              << channel.isInvited(&charlie) << std::endl;

    channel.addInvitedClient(&charlie);

    std::cout << "Charlie invited after duplicate invite: "
              << channel.isInvited(&charlie) << std::endl;

    channel.removeInvitedClient(&charlie);

    std::cout << "Charlie invited after removal: "
              << channel.isInvited(&charlie) << std::endl;


    std::cout << "\n--- +t Topic Restriction ---" << std::endl;

    std::cout << "Topic restricted: "
              << channel.isTopicRestricted() << std::endl;

    channel.changeTopicRestriction();

    std::cout << "Topic restricted after +t: "
              << channel.isTopicRestricted() << std::endl;

    channel.changeTopicRestriction();

    std::cout << "Topic restricted after -t: "
              << channel.isTopicRestricted() << std::endl;


    std::cout << "\n--- +k Channel Key ---" << std::endl;

    std::cout << "Has key: "
              << channel.isKey() << std::endl;

    channel.changeKey("secret123");

    std::cout << "Has key after +k: "
              << channel.isKey() << std::endl;

    std::cout << "Key: "
              << channel.getKey() << std::endl;

    channel.disableKey();

    std::cout << "Has key after -k: "
              << channel.isKey() << std::endl;


    std::cout << "\n--- +l User Limit ---" << std::endl;

    std::cout << "Limit enabled: "
              << channel.isUserLimitEnable() << std::endl;

    channel.changeLimit(50);

    std::cout << "Limit enabled after +l: "
              << channel.isUserLimitEnable() << std::endl;

    std::cout << "User limit: "
              << channel.getLimit() << std::endl;

    channel.changeLimit(100);

    std::cout << "User limit after changing: "
              << channel.getLimit() << std::endl;

    channel.disableLimit();

    std::cout << "Limit enabled after -l: "
              << channel.isUserLimitEnable() << std::endl;


    std::cout << "\n--- Remove Member ---" << std::endl;

    channel.addOp(&bob);

    std::cout << "Bob member before removal: "
              << channel.isMember(&bob) << std::endl;

    std::cout << "Bob operator before removal: "
              << channel.isOp(&bob) << std::endl;

    channel.removeMember(&bob);

    std::cout << "Bob member after removal: "
              << channel.isMember(&bob) << std::endl;

    std::cout << "Bob operator after removal: "
              << channel.isOp(&bob) << std::endl;

    return 0;
}