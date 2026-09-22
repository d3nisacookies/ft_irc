#include "Client.hpp"
#include "Channel.hpp"

#include <iostream>

Channel::Channel(const std::string& name) 
    : _name(name), _inviteOnly(false), _topicRestriction(false), 
        _hasKey(false), _userLimitEnable(false) {}

Channel::~Channel() {}

void Channel::addMember(Client* member) {
    if(member == NULL)
        std::cout << "added member cannot be NULL" << std::endl;
    else
        _members.insert(member);
}

void Channel::removeMember(Client* member) {
    _members.erase(member);
    if (_operators.find(member) != _operators.end())
        _operators.erase(member);
}

bool Channel::isMember(Client* member) {
    if(_members.find(member) != _members.end())
        return true;
    return false;
}


bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}

void Channel::changeInviteStatus()
{
    _inviteOnly = !_inviteOnly;
}

void Channel::addInvitedClient(Client* member)
{
    _invitedMembers.insert(member);
}

void Channel::removeInvitedClient(Client* member)
{
    _invitedMembers.erase(member);
}

bool Channel::isInvited(Client* member)
{
    if(_invitedMembers.find(member) != _invitedMembers.end())
        return true;
    return false;
}


bool Channel::isTopicRestricted() const
{
    return _topicRestriction;
}

void Channel::changeTopicRestriction()
{
    _topicRestriction = !_topicRestriction;
}

bool Channel::isKey() const
{
    return _hasKey;
}

void Channel::changeKey(std::string password)
{
    _key = password;
    _hasKey = true;
}

void Channel::disableKey()
{
    _hasKey = false;
}

bool Channel::isUserLimitEnable() const
{
    return _userLimitEnable;
}

void Channel::changeLimit(long long amount)
{
    if(amount > INT_MAX - 1)
    {
        std::cout << "User limit exceeded." << std::endl;
        return;
    }
    if (amount < 0)
        return;
    _userLimitEnable = true;
    _userLimit = amount;
}

void Channel::disableLimit()
{
    _userLimitEnable = false;
}

void Channel::addOp(Client* op) {
    if(_members.find(op) != _members.end())
        _operators.insert(op);
    else
        std::cout << op->getHost() << " is not a member." << std::endl;
}

void Channel::removeOp(Client* op) {
    _operators.erase(op);
}

bool Channel::isOp(Client* op) {
    if(_operators.find(op) != _operators.end())
        return true;
    return false;
}

void Channel::setTopic(std::string newTopic) {
    _topic = newTopic;
}

const std::string& Channel::getName() const {
    return _name;
}

const std::string& Channel::getTopic() const {
    return _topic;
}

const std::set<Client *>& Channel::getMembers() const {
    return _members;
}

const std::string& Channel::getKey() const
{
    return _key;
}

const int& Channel::getLimit() const
{
    return _userLimit;
}
