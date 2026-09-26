#include "Server.hpp"

void	Server::cmdQuit(Client *client, const Message &msg)
{
	std::string	reason("Client Quit");

	if (!msg.params.empty() && !msg.params[0].empty())
		reason = "Quit: " + msg.params[0];
	disconnectClient(client, reason, true);
}
