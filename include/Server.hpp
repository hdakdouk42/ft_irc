#ifndef SERVER_HPP
# define SERVER_HPP

# include <map>
# include <string>
# include <vector>

# include <poll.h>
# include <signal.h>

# include "Channel.hpp"
# include "Client.hpp"
# include "Message.hpp"

/* How many bytes we try to read per recv(). */
# define READ_BUFFER_SIZE	512

/*
** Hard limit on a single unterminated line. A client that sends more than
** that without a '\n' is dropped instead of letting the buffer grow forever.
*/
# define MAX_LINE_LENGTH	4096

/* Set to 1 by the signal handler so the main loop can exit cleanly. */
extern volatile sig_atomic_t	g_stop;

class Server
{
	public:
		Server(unsigned short port, const std::string &password);
		~Server();

		/* Single poll() loop: accept, read, write, dispatch. */
		void	run();

	private:
		Server(const Server &other);
		Server	&operator=(const Server &other);

		/* ---- setup ---- */
		void		setupSocket();

		/* ---- event loop ---- */
		void		buildPollFds();
		void		acceptClient();
		void		receiveFromClient(Client *client);
		void		sendToClient(Client *client);
		void		handleLine(Client *client, const std::string &line);
		void		dropClosedClients();

		/* ---- client / channel bookkeeping ---- */
		Client		*getClient(int fd) const;
		Client		*findClientByNick(const std::string &nickname) const;
		Channel		*findChannel(const std::string &name) const;
		Channel		*createChannel(const std::string &name);
		void		removeChannel(Channel *channel);
		void		removeChannelIfEmpty(Channel *channel);
		void		removeClientFromChannels(Client *client);
		void		leaveChannel(Channel *channel, Client *client);
		void		disconnectClient(Client *client, const std::string &reason,
						bool graceful);
		void		removeClient(int fd);

		/* ---- sending ---- */
		void		sendTo(Client *client, const std::string &message);
		void		broadcastToChannel(Channel *channel,
						const std::string &message, Client *except);
		void		broadcastToPeers(Client *client,
						const std::string &message);

		/* ---- registration ---- */
		void		tryRegister(Client *client);

		/* ---- command dispatch ---- */
		void		execute(Client *client, const Message &msg);

		/* ---- commands (one file each, in src/commands) ---- */
		void		cmdCap(Client *client, const Message &msg);
		void		cmdPass(Client *client, const Message &msg);
		void		cmdNick(Client *client, const Message &msg);
		void		cmdUser(Client *client, const Message &msg);
		void		cmdPing(Client *client, const Message &msg);
		void		cmdQuit(Client *client, const Message &msg);
		void		cmdJoin(Client *client, const Message &msg);
		void		cmdPart(Client *client, const Message &msg);
		void		cmdPrivmsg(Client *client, const Message &msg, bool notice);
		void		cmdKick(Client *client, const Message &msg);
		void		cmdInvite(Client *client, const Message &msg);
		void		cmdTopic(Client *client, const Message &msg);
		void		cmdMode(Client *client, const Message &msg);
		void		applyChannelModes(Client *client, Channel *channel,
						const Message &msg);

		unsigned short					_port;
		std::string						_password;
		std::string						_createdAt;
		int								_listenFd;
		std::vector<struct pollfd>		_pollFds;
		std::map<int, Client *>			_clients;
		/* key = channel name in IRC lower case, value = the channel */
		std::map<std::string, Channel *>	_channels;
};

#endif
