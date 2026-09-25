#include "Server.hpp"
#include "Replies.hpp"
#include "Utils.hpp"

void	Server::cmdJoin(Client *client, const Message &msg)
{
	const std::string	&nickname = client->getNickname();

	if (msg.params.empty() || msg.params[0].empty())
	{
		sendTo(client, ERR_NEEDMOREPARAMS(nickname, "JOIN"));
		return ;
	}

	std::vector<std::string>	names = utils::split(msg.params[0], ',');
	std::vector<std::string>	keys;

	if (msg.params.size() > 1)
		keys = utils::split(msg.params[1], ',');
	for (std::size_t i = 0; i < names.size(); ++i)
	{
		const std::string	&name = names[i];
		const std::string	key = (i < keys.size()) ? keys[i] : std::string("");

		if (!utils::isValidChannelName(name))
		{
			sendTo(client, ERR_NOSUCHCHANNEL(nickname, name));
			continue ;
		}

		Channel	*channel = findChannel(name);
		bool	justCreated = false;

		if (channel == NULL)
		{
			channel = createChannel(name);
			justCreated = true;
		}
		if (channel->isMember(client))
			continue ;
		if (!justCreated)
		{
			const bool	invited = channel->isInvited(client);

			if (channel->isInviteOnly() && !invited)
			{
				sendTo(client, ERR_INVITEONLYCHAN(nickname, channel->getName()));
				continue ;
			}
			if (channel->hasKey() && channel->getKey() != key)
			{
				sendTo(client, ERR_BADCHANNELKEY(nickname, channel->getName()));
				continue ;
			}
			if (channel->hasUserLimit() && channel->size() >= channel->getUserLimit())
			{
				sendTo(client, ERR_CHANNELISFULL(nickname, channel->getName()));
				continue ;
			}
		}
		channel->addMember(client);
		channel->removeInvite(client);
		if (justCreated)
			channel->addOperator(client);

		broadcastToChannel(channel, ":" + client->getPrefix() + " JOIN " + channel->getName() + "\r\n", NULL);
		if (!channel->getTopic().empty())
			sendTo(client, RPL_TOPIC(nickname, channel->getName(), channel->getTopic()));
		sendTo(client, RPL_NAMREPLY(nickname, channel->getName(), channel->getNamesList()));
		sendTo(client, RPL_ENDOFNAMES(nickname, channel->getName()));
	}
}
