#include "Server.hpp"
#include "Replies.hpp"
#include "Utils.hpp"

static void	appendMode(std::string &flags, char &lastSign, bool adding, char mode)
{
	const char	sign = adding ? '+' : '-';

	if (lastSign != sign)
	{
		flags += sign;
		lastSign = sign;
	}
	flags += mode;
}

void	Server::cmdMode(Client *client, const Message &msg)
{
	const std::string	&nickname = client->getNickname();

	if (msg.params.empty() || msg.params[0].empty())
	{
		sendTo(client, ERR_NEEDMOREPARAMS(nickname, "MODE"));
		return ;
	}
	if (!utils::isChannelName(msg.params[0]))
	{
		if (!utils::ircEqual(msg.params[0], nickname))
			sendTo(client, ERR_USERSDONTMATCH(nickname));
		else if (msg.params.size() == 1)
			sendTo(client, RPL_UMODEIS(nickname));
		return ;
	}

	Channel	*channel = findChannel(msg.params[0]);

	if (channel == NULL)
	{
		sendTo(client, ERR_NOSUCHCHANNEL(nickname, msg.params[0]));
		return ;
	}

	if (msg.params.size() == 1)
	{
		sendTo(client, RPL_CHANNELMODEIS(nickname, channel->getName(),
				channel->getModeString(channel->isMember(client))));
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
	applyChannelModes(client, channel, msg);
}

void	Server::applyChannelModes(Client *client, Channel *channel, const Message &msg)
{
	const std::string	&nickname = client->getNickname();
	const std::string	&modes = msg.params[1];
	std::size_t			argIndex = 2;
	bool				adding = true;
	std::string			appliedFlags;
	std::string			appliedArgs;
	char				lastSign = 0;
	Client				*promoted = NULL;

	for (std::size_t i = 0; i < modes.size(); ++i)
	{
		const char	mode = modes[i];

		if (mode == '+' || mode == '-')
		{
			adding = (mode == '+');
			continue ;
		}
		if (mode == 'i')
		{
			channel->setInviteOnly(adding);
			appendMode(appliedFlags, lastSign, adding, mode);
		}
		else if (mode == 't')
		{
			channel->setTopicRestricted(adding);
			appendMode(appliedFlags, lastSign, adding, mode);
		}
		else if (mode == 'k')
		{
			if (adding)
			{
				if (argIndex >= msg.params.size())
				{
					sendTo(client, ERR_NEEDMOREPARAMS(nickname, "MODE"));
					continue ;
				}

				const std::string	key = msg.params[argIndex++];

				if (key.empty() || key.find(' ') != std::string::npos || key.find(',') != std::string::npos)
				{
					sendTo(client, ERR_NEEDMOREPARAMS(nickname, "MODE"));
					continue ;
				}
				channel->setKey(key);
				appendMode(appliedFlags, lastSign, adding, mode);
				appliedArgs += " " + key;
			}
			else
			{
				if (argIndex < msg.params.size())
					++argIndex;
				channel->removeKey();
				appendMode(appliedFlags, lastSign, adding, mode);
			}
		}
		else if (mode == 'l')
		{
			if (adding)
			{
				unsigned long	limit = 0;

				if (argIndex >= msg.params.size())
				{
					sendTo(client, ERR_NEEDMOREPARAMS(nickname, "MODE"));
					continue ;
				}
				if (!utils::parsePositiveNumber(msg.params[argIndex++], limit))
					continue ;
				channel->setUserLimit(static_cast<std::size_t>(limit));
				appendMode(appliedFlags, lastSign, adding, mode);
				appliedArgs += " " + utils::toString(static_cast<long>(limit));
			}
			else
			{
				channel->removeUserLimit();
				appendMode(appliedFlags, lastSign, adding, mode);
			}
		}
		else if (mode == 'o')
		{
			if (argIndex >= msg.params.size())
			{
				sendTo(client, ERR_NEEDMOREPARAMS(nickname, "MODE"));
				continue ;
			}

			const std::string	targetNick = msg.params[argIndex++];
			Client				*target = findClientByNick(targetNick);

			if (target == NULL || !channel->isMember(target))
			{
				sendTo(client, ERR_USERNOTINCHANNEL(nickname, targetNick, channel->getName()));
				continue ;
			}
			if (adding)
				channel->addOperator(target);
			else
			{
				channel->removeOperator(target);
				Client	*next = channel->promoteIfNoOperator(target);

				if (next != NULL)
					promoted = next;
			}
			appendMode(appliedFlags, lastSign, adding, mode);
			appliedArgs += " " + target->getNickname();
		}
		else
			sendTo(client, ERR_UNKNOWNMODE(nickname, std::string(1, mode)));
	}
	if (!appliedFlags.empty())
		broadcastToChannel(channel, ":" + client->getPrefix() + " MODE " + channel->getName() + " " + appliedFlags + appliedArgs + "\r\n", NULL);

	if (promoted != NULL)
		broadcastToChannel(channel, ":" SERVER_NAME " MODE " + channel->getName() + " +o " + promoted->getNickname() + "\r\n", NULL);
}
