#include "Server.hpp"
#include "Replies.hpp"
#include "Utils.hpp"

void	Server::cmdCap(Client *client, const Message &msg)
{
	std::string	subcommand;
	std::string	nickname = client->getNickOrStar();

	if (msg.params.empty())
		return ;
	subcommand = utils::toUpper(msg.params[0]);
	if (subcommand == "LS")
		sendTo(client, ":" SERVER_NAME " CAP " + nickname + " LS :\r\n");
	else if (subcommand == "LIST")
		sendTo(client, ":" SERVER_NAME " CAP " + nickname + " LIST :\r\n");
	else if (subcommand == "REQ")
	{
		std::string	requested;

		if (msg.params.size() > 1)
			requested = msg.params[1];
		sendTo(client, ":" SERVER_NAME " CAP " + nickname + " NAK :" + requested + "\r\n");
	}
}
