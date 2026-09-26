#include "Server.hpp"
#include "Replies.hpp"
#include "Utils.hpp"

void	Server::cmdPart(Client *client, const Message &msg)
{
	const std::string	&nickname = client->getNickname();

	if (msg.params.empty() || msg.params[0].empty())
	{
		sendTo(client, ERR_NEEDMOREPARAMS(nickname, "PART"));
		return ;
	}

	std::vector<std::string>	names = utils::split(msg.params[0], ',');
	std::string					reason;

	if (msg.params.size() > 1)
		reason = msg.params[1];
	for (std::size_t i = 0; i < names.size(); ++i)
	{
		Channel	*channel = findChannel(names[i]);

		if (channel == NULL)
		{
			sendTo(client, ERR_NOSUCHCHANNEL(nickname, names[i]));
			continue ;
		}
		if (!channel->isMember(client))
		{
			sendTo(client, ERR_NOTONCHANNEL(nickname, channel->getName()));
			continue ;
		}

		std::string	notification = ":" + client->getPrefix() + " PART " + channel->getName();

		if (!reason.empty())
			notification += " :" + reason;
		notification += "\r\n";
		broadcastToChannel(channel, notification, NULL);
		leaveChannel(channel, client);
		removeChannelIfEmpty(channel);
	}
}
