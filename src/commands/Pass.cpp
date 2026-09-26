#include "Server.hpp"
#include "Replies.hpp"

void	Server::cmdPass(Client *client, const Message &msg)
{
	if (client->isRegistered())
	{
		sendTo(client, ERR_ALREADYREGISTRED(client->getNickOrStar()));
		return ;
	}
	if (msg.params.empty() || msg.params[0].empty())
	{
		sendTo(client, ERR_NEEDMOREPARAMS(client->getNickOrStar(), "PASS"));
		return ;
	}
	if (msg.params[0] != _password)
	{
		sendTo(client, ERR_PASSWDMISMATCH(client->getNickOrStar()));
		disconnectClient(client, "Access denied: incorrect password", true);
		return ;
	}
	client->setPassword(true);
}
