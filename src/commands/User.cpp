#include "Server.hpp"
#include "Replies.hpp"

void	Server::cmdUser(Client *client, const Message &msg)
{
	if (client->isRegistered())
	{
		sendTo(client, ERR_ALREADYREGISTRED(client->getNickOrStar()));
		return ;
	}
	if (msg.params.size() < 4 || msg.params[0].empty())
	{
		sendTo(client, ERR_NEEDMOREPARAMS(client->getNickOrStar(), "USER"));
		return ;
	}
	client->setUser(msg.params[0], msg.params[3]);
	tryRegister(client);
}
