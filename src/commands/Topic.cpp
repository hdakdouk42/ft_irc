#include "Server.hpp"
#include "Replies.hpp"

void	Server::cmdTopic(Client *client, const Message &msg)
{
	const std::string	&nickname = client->getNickname();

	if (msg.params.empty() || msg.params[0].empty())
	{
		sendTo(client, ERR_NEEDMOREPARAMS(nickname, "TOPIC"));
		return ;
	}

	Channel	*channel = findChannel(msg.params[0]);

	if (channel == NULL)
	{
		sendTo(client, ERR_NOSUCHCHANNEL(nickname, msg.params[0]));
		return ;
	}
	if (!channel->isMember(client))
	{
		sendTo(client, ERR_NOTONCHANNEL(nickname, channel->getName()));
		return ;
	}
	if (msg.params.size() < 2)
	{
		if (channel->getTopic().empty())
			sendTo(client, RPL_NOTOPIC(nickname, channel->getName()));
		else
			sendTo(client, RPL_TOPIC(nickname, channel->getName(), channel->getTopic()));
		return ;
	}
	if (channel->isTopicRestricted() && !channel->isOperator(client))
	{
		sendTo(client, ERR_CHANOPRIVSNEEDED(nickname, channel->getName()));
		return ;
	}
	channel->setTopic(msg.params[1]);
	broadcastToChannel(channel, ":" + client->getPrefix() + " TOPIC " + channel->getName() + " :" + channel->getTopic() + "\r\n", NULL);
}
