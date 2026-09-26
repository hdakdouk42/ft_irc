#include "Channel.hpp"
#include "Client.hpp"
#include "Utils.hpp"

#include <algorithm>

static	void	eraseClient(std::vector<Client *> &list, Client *client)
{
	std::vector<Client *>::iterator	it = std::find(list.begin(), list.end(), client);

	if (it != list.end())
		list.erase(it);
}

static	bool	containsClient(const std::vector<Client *> &list, Client *client)
{
	return (std::find(list.begin(), list.end(), client) != list.end());
}

Channel::Channel(const std::string &name)
	: _name(name),
	  _topic(""),
	  _inviteOnly(false),
	  _topicRestricted(false),
	  _hasKey(false),
	  _key(""),
	  _hasUserLimit(false),
	  _userLimit(0){}

Channel::~Channel() {}

const	std::string	&Channel::getName() const { return (_name); }

const	std::string	&Channel::getTopic() const { return (_topic); }

void	Channel::setTopic(const std::string &topic) { _topic = topic; }

void	Channel::addMember(Client *client)
{
	if (!containsClient(_members, client))
		_members.push_back(client);
}

Client	*Channel::removeMember(Client *client)
{
	if (!isMember(client))
		return (NULL);
	eraseClient(_members, client);
	eraseClient(_operators, client);
	eraseClient(_invited, client);
	return (promoteIfNoOperator(NULL));
}

bool	Channel::isMember(Client *client) const
{
	return (containsClient(_members, client));
}

bool	Channel::isEmpty() const { return (_members.empty()); }

std::size_t	Channel::size() const { return (_members.size()); }

const std::vector<Client *>	&Channel::getMembers() const { return (_members); }

void	Channel::addOperator(Client *client)
{
	if (isMember(client) && !containsClient(_operators, client))
		_operators.push_back(client);
}

void	Channel::removeOperator(Client *client) { eraseClient(_operators, client); }

Client	*Channel::promoteIfNoOperator(Client *except)
{
	if (!_operators.empty())
		return (NULL);
	for (std::size_t i = 0; i < _members.size(); ++i)
	{
		if (_members[i] != except)
		{
			_operators.push_back(_members[i]);
			return (_members[i]);
		}
	}
	return (NULL);
}

bool	Channel::isOperator(Client *client) const
{
	return (containsClient(_operators, client));
}

void	Channel::addInvite(Client *client)
{
	if (!containsClient(_invited, client))
		_invited.push_back(client);
}

void	Channel::removeInvite(Client *client)
{
	eraseClient(_invited, client);
}

bool	Channel::isInvited(Client *client) const
{
	return (containsClient(_invited, client));
}

/* ------------------------------ modes ----------------------------------- */

bool	Channel::isInviteOnly() const { return (_inviteOnly); }

void	Channel::setInviteOnly(bool value) { _inviteOnly = value; }

bool	Channel::isTopicRestricted() const { return (_topicRestricted); }

void	Channel::setTopicRestricted(bool value) {_topicRestricted = value; }

bool	Channel::hasKey() const { return (_hasKey); }

const std::string	&Channel::getKey() const { return (_key); }

void	Channel::setKey(const std::string &key)
{
	_key = key;
	_hasKey = true;
}

void	Channel::removeKey()
{
	_key.clear();
	_hasKey = false;
}

bool	Channel::hasUserLimit() const { return (_hasUserLimit); }

std::size_t	Channel::getUserLimit() const { return (_userLimit); }

void	Channel::setUserLimit(std::size_t limit)
{
	_userLimit = limit;
	_hasUserLimit = true;
}

void	Channel::removeUserLimit()
{
	_userLimit = 0;
	_hasUserLimit = false;
}

std::string	Channel::getModeString(bool withArguments) const
{
	std::string flags("");
	std::string arguments("");

	if (_inviteOnly) flags += "i";
	if (_topicRestricted) flags += "t";
	if (_hasKey)
	{
		flags += "k";
		if (withArguments) arguments += " " + _key;
	}
	if (_hasUserLimit)
	{
		flags += "l";
		if (withArguments) arguments += " " + utils::toString(static_cast<long>(_userLimit));
	}
	if (flags.empty())
		return ("");
	return ("+" + flags + arguments);
}

std::string	Channel::getNamesList() const
{
	std::string	names("");

	for (std::size_t i = 0; i < _members.size(); ++i)
	{
		if (i != 0)
			names += " ";
		if (containsClient(_operators, _members[i]))
			names += "@";
		names += _members[i]->getNickname();
	}
	return (names);
}
