#include "Server.hpp"
#include "Replies.hpp"
#include "Utils.hpp"

void	Server::cmdPrivmsg(Client *client, const Message &msg, bool notice)
{
	const std::string	&nickname = client->getNickname();
	const std::string	command = notice ? "NOTICE" : "PRIVMSG";

	if (msg.params.empty() || msg.params[0].empty())
	{
		if (!notice)
			sendTo(client, ERR_NORECIPIENT(nickname, command));
		return ;
	}
	if (msg.params.size() < 2 || msg.params[1].empty())
	{
		if (!notice)
			sendTo(client, ERR_NOTEXTTOSEND(nickname));
		return ;
	}

	std::vector<std::string>	targets = utils::split(msg.params[0], ',');
	const std::string			&text = msg.params[1];

	for (std::size_t i = 0; i < targets.size(); ++i)
	{
		const std::string	&target = targets[i];

		if (target.empty())
			continue ;
		if (utils::isChannelName(target))
		{
			Channel	*channel = findChannel(target);

			if (channel == NULL)
			{
				if (!notice)
					sendTo(client, ERR_NOSUCHCHANNEL(nickname, target));
				continue ;
			}
			if (!channel->isMember(client))
			{
				if (!notice)
					sendTo(client, ERR_CANNOTSENDTOCHAN(nickname, channel->getName()));
				continue ;
			}
			broadcastToChannel(channel, ":" + client->getPrefix() + " " + command + " " + channel->getName() + " :" + text + "\r\n", client);
		}
		else
		{
			Client	*receiver = findClientByNick(target);

			if (receiver == NULL)
			{
				if (!notice)
					sendTo(client, ERR_NOSUCHNICK(nickname, target));
				continue ;
			}
			sendTo(receiver, ":" + client->getPrefix() + " " + command + " " + receiver->getNickname() + " :" + text + "\r\n");
		}
	}
}
