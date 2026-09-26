#include "Server.hpp"
#include "Replies.hpp"

void	Server::cmdInvite(Client *client, const Message &msg)
{
	const std::string	&nickname = client->getNickname();

	if (msg.params.size() < 2 || msg.params[0].empty()
		|| msg.params[1].empty())
	{
		sendTo(client, ERR_NEEDMOREPARAMS(nickname, "INVITE"));
		return ;
	}

	Client	*target = findClientByNick(msg.params[0]);

	if (target == NULL)
	{
		sendTo(client, ERR_NOSUCHNICK(nickname, msg.params[0]));
		return ;
	}

	Channel	*channel = findChannel(msg.params[1]);

	if (channel == NULL)
	{
		sendTo(client, ERR_NOSUCHCHANNEL(nickname, msg.params[1]));
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
	if (channel->isMember(target))
	{
		sendTo(client, ERR_USERONCHANNEL(nickname, target->getNickname(), channel->getName()));
		return ;
	}
	channel->addInvite(target);
	sendTo(client, RPL_INVITING(nickname, target->getNickname(), channel->getName()));
	sendTo(target, ":" + client->getPrefix() + " INVITE " + target->getNickname() + " :" + channel->getName() + "\r\n");
}
