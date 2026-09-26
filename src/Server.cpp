#include "Server.hpp"
#include "Replies.hpp"
#include "Utils.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

#include <csignal>
#include <cstring>
#include <ctime>
#include <iostream>
#include <stdexcept>

volatile sig_atomic_t	g_stop = 0;

static void	signalHandler(int signum)
{
	(void)signum;
	g_stop = 1;
}

/* ------------------------------------------------------------------------ */
/*                            construction                                  */
/* ------------------------------------------------------------------------ */

Server::Server(unsigned short port, const std::string &password)
	: _port(port),
	  _password(password),
	  _createdAt(""),
	  _listenFd(-1)
{
	std::time_t	now = std::time(NULL);
	std::string	date(std::ctime(&now));

	if (!date.empty() && date[date.size() - 1] == '\n')
		date.erase(date.size() - 1);
	_createdAt = date;
	setupSocket();
}

Server::~Server()
{
	for (std::map<int, Client *>::iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		close(it->second->getFd());
		delete it->second;
	}
	_clients.clear();
	for (std::map<std::string, Channel *>::iterator it = _channels.begin();
		it != _channels.end(); ++it)
		delete it->second;
	_channels.clear();
	if (_listenFd >= 0)
		close(_listenFd);
}

/*
** Creates the listening socket:
**   - SO_REUSEADDR so the port can be reused right after a restart
**   - O_NONBLOCK because the subject forbids any blocking I/O
*/
void	Server::setupSocket()
{
	struct sockaddr_in	address;
	int					option = 1;

	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd < 0)
		throw std::runtime_error("socket() failed");
	if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &option,
			sizeof(option)) < 0)
	{
		close(_listenFd);
		_listenFd = -1;
		throw std::runtime_error("setsockopt() failed");
	}
	if (fcntl(_listenFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(_listenFd);
		_listenFd = -1;
		throw std::runtime_error("fcntl() failed");
	}
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(_port);
	if (bind(_listenFd, reinterpret_cast<struct sockaddr *>(&address),
			sizeof(address)) < 0)
	{
		close(_listenFd);
		_listenFd = -1;
		throw std::runtime_error("bind() failed (is the port already in use?)");
	}
	if (listen(_listenFd, SOMAXCONN) < 0)
	{
		close(_listenFd);
		_listenFd = -1;
		throw std::runtime_error("listen() failed");
	}
}

/* ------------------------------------------------------------------------ */
/*                              event loop                                  */
/* ------------------------------------------------------------------------ */

/*
** Rebuilt before every poll() call.
** POLLIN  is watched for every client that may still send something.
** POLLOUT is watched only when there is something left to write, so poll()
**         never wakes up for nothing.
*/
void	Server::buildPollFds()
{
	struct pollfd	entry;

	_pollFds.clear();
	entry.fd = _listenFd;
	entry.events = POLLIN;
	entry.revents = 0;
	_pollFds.push_back(entry);
	for (std::map<int, Client *>::const_iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		Client	*client = it->second;

		if (client->isDead())
			continue ;
		entry.fd = client->getFd();
		entry.events = 0;
		if (!client->isClosing())
			entry.events |= POLLIN;
		if (!client->outBuffer().empty())
			entry.events |= POLLOUT;
		entry.revents = 0;
		_pollFds.push_back(entry);
	}
}

void	Server::run()
{
	std::signal(SIGPIPE, SIG_IGN);
	std::signal(SIGINT, signalHandler);
	std::signal(SIGQUIT, signalHandler);
	std::signal(SIGTERM, signalHandler);

	std::cout << SERVER_NAME << " listening on port " << _port << std::endl;
	while (g_stop == 0)
	{
		buildPollFds();
		if (poll(&_pollFds[0], _pollFds.size(), -1) < 0)
		{
			/*
			** With a valid set of descriptors the only realistic failure is
			** an interruption by a signal, so the set is simply rebuilt and
			** poll() is called again. No errno is inspected anywhere in this
			** server, and in particular never to decide to read or write
			** again outside of poll().
			** A shutdown signal is handled by the loop condition above.
			*/
			continue ;
		}
		for (std::size_t i = 0; i < _pollFds.size(); ++i)
		{
			const short	revents = _pollFds[i].revents;
			const int	fd = _pollFds[i].fd;

			if (revents == 0)
				continue ;
			if (fd == _listenFd)
			{
				if (revents & POLLIN)
					acceptClient();
				continue ;
			}

			Client	*client = getClient(fd);

			if (client == NULL)
				continue ;
			if (revents & POLLOUT)
				sendToClient(client);
			if (!client->isDead() && (revents & POLLIN))
				receiveFromClient(client);
			if (!client->isDead() && (revents & (POLLHUP | POLLERR | POLLNVAL)))
				disconnectClient(client, "Connection reset by peer", false);
		}
		dropClosedClients();
	}
	std::cout << std::endl << "Shutting down " << SERVER_NAME << "..."
		<< std::endl;
}

void	Server::acceptClient()
{
	struct sockaddr_in	address;
	socklen_t			length = sizeof(address);
	int					fd;

	std::memset(&address, 0, sizeof(address));
	fd = accept(_listenFd, reinterpret_cast<struct sockaddr *>(&address),
			&length);
	if (fd < 0)
		return ;
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(fd);
		return ;
	}
	try
	{
		Client	*client = new Client(fd,
				std::string(inet_ntoa(address.sin_addr)));

		_clients[fd] = client;
		std::cout << "* client connected: fd " << fd << " from "
			<< client->getHost() << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: could not register a new client: " << e.what()
			<< std::endl;
		close(fd);
	}
}

/*
** recv() is only ever called after poll() reported POLLIN on that fd.
** The bytes are appended to the client buffer, then every complete line
** ('\n' terminated) is extracted and executed. What is left is kept for
** the next round: this is what makes "com" + "man" + "d\n" work.
*/
void	Server::receiveFromClient(Client *client)
{
	char	buffer[READ_BUFFER_SIZE];
	ssize_t	bytes;

	bytes = recv(client->getFd(), buffer, sizeof(buffer), 0);
	if (bytes <= 0)
	{
		disconnectClient(client, "Connection closed", false);
		return ;
	}
	client->inBuffer().append(buffer, static_cast<std::size_t>(bytes));
	while (!client->isDead() && !client->isClosing())
	{
		const std::string::size_type	pos = client->inBuffer().find('\n');

		if (pos == std::string::npos)
			break ;

		std::string	line = client->inBuffer().substr(0, pos);

		client->inBuffer().erase(0, pos + 1);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (!line.empty())
			handleLine(client, line);
	}
	if (client->inBuffer().size() > MAX_LINE_LENGTH)
		disconnectClient(client, "Input line too long", true);
}

/*
** send() is only ever called after poll() reported POLLOUT on that fd.
** A partial write is fine: whatever was not sent stays in the buffer and
** POLLOUT will be requested again on the next loop.
*/
void	Server::sendToClient(Client *client)
{
	std::string	&out = client->outBuffer();
	ssize_t		bytes;

	if (out.empty())
		return ;
	bytes = send(client->getFd(), out.c_str(), out.size(), 0);
	if (bytes <= 0)
	{
		disconnectClient(client, "Write error", false);
		return ;
	}
	out.erase(0, static_cast<std::size_t>(bytes));
}

void	Server::handleLine(Client *client, const std::string &line)
{
	const Message	msg = parseMessage(line);

	if (msg.command.empty())
		return ;
	std::cout << "<< [fd " << client->getFd() << "] " << msg.command;
	if (msg.command != "PASS")
	{
		for (std::size_t i = 0; i < msg.params.size(); ++i)
			std::cout << " " << msg.params[i];
	}
	else
		std::cout << " ********";
	std::cout << std::endl;
	try
	{
		execute(client, msg);
	}
	catch (const std::exception &e)
	{
		/* The server must survive anything, including a failed allocation. */
		std::cerr << "Error: while executing " << msg.command << ": "
			<< e.what() << std::endl;
	}
}

/*
** Closes every client that is finished:
**   DEAD    -> nothing left to say
**   CLOSING -> its farewell message has been flushed
*/
void	Server::dropClosedClients()
{
	std::vector<int>	finished;

	for (std::map<int, Client *>::const_iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		Client	*client = it->second;

		if (client->isDead()
			|| (client->isClosing() && client->outBuffer().empty()))
			finished.push_back(it->first);
	}
	for (std::size_t i = 0; i < finished.size(); ++i)
		removeClient(finished[i]);
}
