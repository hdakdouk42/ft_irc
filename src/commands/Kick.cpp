#include "Server.hpp"
#include "Replies.hpp"
#include "Utils.hpp"

void	Server::cmdKick(Client *client, const Message &msg)
{
	const std::string	&nickname = client->getNickname();

	if (msg.params.size() < 2 || msg.params[0].empty()
		|| msg.params[1].empty())
	{
		sendTo(client, ERR_NEEDMOREPARAMS(nickname, "KICK"));
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
	if (!channel->isOperator(client))
	{
		sendTo(client, ERR_CHANOPRIVSNEEDED(nickname, channel->getName()));
		return ;
	}

	std::vector<std::string>	targets = utils::split(msg.params[1], ',');
	std::string					reason = nickname;

	if (msg.params.size() > 2 && !msg.params[2].empty())
		reason = msg.params[2];
	for (std::size_t i = 0; i < targets.size(); ++i)
	{
		Client	*victim = findClientByNick(targets[i]);

		if (victim == NULL || !channel->isMember(victim))
		{
			sendTo(client, ERR_USERNOTINCHANNEL(nickname, targets[i],
					channel->getName()));
			continue ;
		}
		broadcastToChannel(channel, ":" + client->getPrefix() + " KICK "
			+ channel->getName() + " " + victim->getNickname() + " :" + reason
			+ "\r\n", NULL);
		leaveChannel(channel, victim);
		if (channel->isEmpty())
		{
			removeChannel(channel);
			return ;
		}
	}
}
