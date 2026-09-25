#include "Server.hpp"
#include "Replies.hpp"

void	Server::cmdPing(Client *client, const Message &msg)
{
	if (msg.params.empty() || msg.params[0].empty())
	{
		sendTo(client, ERR_NOORIGIN(client->getNickOrStar()));
		return ;
	}
	sendTo(client, ":" SERVER_NAME " PONG " SERVER_NAME " :" + msg.params[0] + "\r\n");
}
