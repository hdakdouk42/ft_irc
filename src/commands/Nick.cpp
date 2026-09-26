#include "Server.hpp"
#include "Replies.hpp"
#include "Utils.hpp"

void	Server::cmdNick(Client *client, const Message &msg)
{
	Client	*owner;

	if (msg.params.empty() || msg.params[0].empty())
	{
		sendTo(client, ERR_NONICKNAMEGIVEN(client->getNickOrStar()));
		return ;
	}

	const std::string	&wanted = msg.params[0];

	if (!utils::isValidNickname(wanted))
	{
		sendTo(client, ERR_ERRONEUSNICKNAME(client->getNickOrStar(), wanted));
		return ;
	}
	owner = findClientByNick(wanted);
	if (owner != NULL && owner != client)
	{
		sendTo(client, ERR_NICKNAMEINUSE(client->getNickOrStar(), wanted));
		return ;
	}
	if (client->getNickname() == wanted)
		return ;
	if (client->isRegistered())
	{
		const std::string	notification = ":" + client->getPrefix() + " NICK :" + wanted + "\r\n";
		sendTo(client, notification);
		broadcastToPeers(client, notification);
		client->setNickname(wanted);
	}
	else
	{
		client->setNickname(wanted);
		tryRegister(client);
	}
}
