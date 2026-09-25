#include "Server.hpp"
#include "Replies.hpp"
#include "Utils.hpp"

#include <unistd.h>

#include <iostream>
#include <set>

Client	*Server::getClient(int fd) const //roudy
{
	std::map<int, Client *>::const_iterator	it = _clients.find(fd);

	if (it == _clients.end())
		return (NULL);
	return (it->second);
}

Client	*Server::findClientByNick(const std::string &nickname) const //hdakdouk
{
	std::map<int, Client *>::const_iterator it;

	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->isDead())
			continue ;
		if (utils::ircEqual(it->second->getNickname(), nickname))
			return (it->second);
	}
	return (NULL);
}

Channel	*Server::findChannel(const std::string &name) const //hdakdouk
{
	std::map<std::string, Channel *>::const_iterator	it;

	it = _channels.find(utils::ircToLower(name));
	if (it == _channels.end())
		return (NULL);
	return (it->second);
}

Channel	*Server::createChannel(const std::string &name) //hdakdouk
{
	Channel	*channel = new Channel(name);

	_channels[utils::ircToLower(name)] = channel;
	return (channel);
}

void	Server::removeChannel(Channel *channel) //hdakdouk
{
	std::map<std::string, Channel *>::iterator	it;

	if (channel == NULL)
		return ;
	it = _channels.find(utils::ircToLower(channel->getName()));
	if (it == _channels.end())
		return ;
	delete it->second;
	_channels.erase(it);
}

void	Server::removeChannelIfEmpty(Channel *channel) //hdakdouk
{
	if (channel != NULL && channel->isEmpty())
		removeChannel(channel);
}

void	Server::leaveChannel(Channel *channel, Client *client) //hdakdouk
{
	Client	*promoted = channel->removeMember(client);

	if (promoted != NULL)
		broadcastToChannel(channel, ":" SERVER_NAME " MODE "
			+ channel->getName() + " +o " + promoted->getNickname()
			+ "\r\n", NULL);
}

void	Server::removeClientFromChannels(Client *client) //hdakdouk
{
	std::map<std::string, Channel *>::iterator	it = _channels.begin();

	while (it != _channels.end())
	{
		Channel	*channel = it->second;

		leaveChannel(channel, client);
		if (channel->isEmpty())
		{
			std::map<std::string, Channel *>::iterator	dead = it;

			++it;
			delete dead->second;
			_channels.erase(dead);
		}
		else
			++it;
	}
}

void	Server::disconnectClient(Client *client, const std::string &reason, bool graceful) //roudy
{
	if (client == NULL || client->isDead())
		return ;
	if (client->isAlive())
	{
		if (client->isRegistered())
			broadcastToPeers(client,
				":" + client->getPrefix() + " QUIT :" + reason + "\r\n");
		removeClientFromChannels(client);
		if (graceful)
		{
			client->appendOut("ERROR :Closing link: " + reason + "\r\n");
			client->setState(Client::CLOSING);
			return ;
		}
	}
	client->setState(Client::DEAD);
}

void	Server::removeClient(int fd) //roudy
{
	std::map<int, Client *>::iterator	it = _clients.find(fd);

	if (it == _clients.end())
		return ;
	removeClientFromChannels(it->second);
	std::cout << "* client disconnected: fd " << fd << std::endl;
	close(fd);
	delete it->second;
	_clients.erase(it);
}

void	Server::sendTo(Client *client, const std::string &message) //roudy
{
	if (client == NULL || client->isDead())
		return ;
	client->appendOut(message);
}

void	Server::broadcastToChannel(Channel *channel, const std::string &message, Client *except) //hdakdouk
{
	const std::vector<Client *>	&members = channel->getMembers();

	for (std::size_t i = 0; i < members.size(); ++i)
	{
		if (members[i] != except)
			sendTo(members[i], message);
	}
}

void	Server::broadcastToPeers(Client *client, const std::string &message) //hdakdouk
{
	std::set<Client *>	alreadySent;
	std::map<std::string, Channel *>::const_iterator it;

	for (it = _channels.begin(); it != _channels.end(); ++it)
	{
		Channel	*channel = it->second;

		if (!channel->isMember(client))
			continue ;

		const std::vector<Client *>	&members = channel->getMembers();

		for (std::size_t i = 0; i < members.size(); ++i)
		{
			if (members[i] == client)
				continue ;
			if (alreadySent.insert(members[i]).second)
				sendTo(members[i], message);
		}
	}
}

void	Server::tryRegister(Client *client) //hdakdouk
{
	if (client->isRegistered() || !client->isAlive())
		return ;
	if (client->getNickname().empty() || !client->hasUserInfo())
		return ;
	if (!client->hasPassword())
	{
		sendTo(client, ERR_PASSWDMISMATCH(client->getNickOrStar()));
		disconnectClient(client, "Access denied: password required", true);
		return ;
	}
	client->setRegistered(true);
	sendTo(client, RPL_WELCOME(client->getNickname(), client->getPrefix()));
	sendTo(client, RPL_YOURHOST(client->getNickname()));
	sendTo(client, RPL_CREATED(client->getNickname(), _createdAt));
	sendTo(client, RPL_MYINFO(client->getNickname()));
	sendTo(client, RPL_ISUPPORT(client->getNickname()));
	sendTo(client, ERR_NOMOTD(client->getNickname()));
	std::cout << "* " << client->getNickname() << " registered (fd "
		<< client->getFd() << ")" << std::endl;
}

void	Server::execute(Client *client, const Message &msg) //hdakdouk
{
	const std::string	&command = msg.command;

	if (command == "CAP")
		return (cmdCap(client, msg));
	if (command == "PASS")
		return (cmdPass(client, msg));
	if (command == "NICK")
		return (cmdNick(client, msg));
	if (command == "USER")
		return (cmdUser(client, msg));
	if (command == "QUIT")
		return (cmdQuit(client, msg));
	if (command == "PING")
		return (cmdPing(client, msg));
	if (command == "PONG")
		return ;

	if (!client->isRegistered())
	{
		sendTo(client, ERR_NOTREGISTERED(client->getNickOrStar()));
		return ;
	}

	if (command == "JOIN")
		cmdJoin(client, msg);
	else if (command == "PART")
		cmdPart(client, msg);
	else if (command == "PRIVMSG")
		cmdPrivmsg(client, msg, false);
	else if (command == "NOTICE")
		cmdPrivmsg(client, msg, true);
	else if (command == "KICK")
		cmdKick(client, msg);
	else if (command == "INVITE")
		cmdInvite(client, msg);
	else if (command == "TOPIC")
		cmdTopic(client, msg);
	else if (command == "MODE")
		cmdMode(client, msg);
	else
		sendTo(client, ERR_UNKNOWNCOMMAND(client->getNickname(), command));
}
