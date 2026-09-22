#pragma once

#include "Client.hpp"

#include <string>
#include <set>
#include <climits>

class Channel {
    private:
        std::string _name;

        std::set<Client *> _members;
        std::string _topic;
        
        // +i
        bool _inviteOnly;
        std::set<Client *> _invitedMembers;

        // +t
        bool _topicRestriction;
        
        // +k 
        std::string _key;
        bool _hasKey;

        // +l
        int _userLimit;
        bool _userLimitEnable; // no limit

        // +o
        std::set<Client *> _operators;


    public:
        Channel(const std::string& name);
        ~Channel();

        void addMember(Client* member);
        void removeMember(Client* member);
        bool isMember(Client* member);

        // i
        bool isInviteOnly() const;
        void changeInviteStatus();
        
        void addInvitedClient(Client* member);
        void removeInvitedClient(Client* member);
        bool isInvited(Client* member);

        // t
        bool isTopicRestricted() const;
        void changeTopicRestriction();
        
        // k
        bool isKey() const;
        void changeKey(std::string password);
        void disableKey();

        // l
        bool isUserLimitEnable() const;
        void changeLimit(long long amount);
        void disableLimit();

        // o
        void addOp(Client* op);
        void removeOp(Client* op);
        bool isOp(Client* op);
               
        // setter
        void setTopic(std::string newTopic);

        // getter
        const std::string& getName() const;
        const std::string& getTopic() const;
        const std::set<Client *>& getMembers() const;
        const std::string& getKey() const;
        const int& getLimit() const;
};

