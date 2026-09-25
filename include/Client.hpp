#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>

/*
** One connected socket.
**
** Because every file descriptor is non blocking, a client owns two buffers:
**   _in   raw bytes received so far, sliced into complete lines by the server
**   _out  bytes waiting to be written, flushed only when poll() says POLLOUT
*/
class Client
{
	public:
		enum State
		{
			CONNECTED,	/* normal life                                    */
			CLOSING,	/* flush what is left in _out, then close         */
			DEAD		/* close right away, nothing left to send         */
		};

		Client(int fd, const std::string &host);
		~Client();

		/* ---- identity ---- */
		int					getFd() const;
		const std::string	&getHost() const;
		const std::string	&getNickname() const;
		const std::string	&getUsername() const;
		const std::string	&getRealname() const;

		/* Nickname, or "*" while the client has no nickname yet. */
		std::string			getNickOrStar() const;

		/* "nick!user@host", used as the prefix of every relayed message. */
		std::string			getPrefix() const;

		void				setNickname(const std::string &nickname);
		void				setUser(const std::string &username,
								const std::string &realname);

		/* ---- registration state ---- */
		bool				hasPassword() const;
		void				setPassword(bool value);
		bool				hasUserInfo() const;
		bool				isRegistered() const;
		void				setRegistered(bool value);

		/* ---- connection state ---- */
		State				getState() const;
		void				setState(State state);
		bool				isClosing() const;
		bool				isDead() const;
		bool				isAlive() const;

		/* ---- buffers ---- */
		std::string			&inBuffer();
		std::string			&outBuffer();
		void				appendOut(const std::string &data);

	private:
		Client(const Client &other);
		Client	&operator=(const Client &other);

		int			_fd;
		std::string	_host;
		std::string	_nickname;
		std::string	_username;
		std::string	_realname;
		std::string	_in;
		std::string	_out;
		bool		_hasPassword;
		bool		_hasUserInfo;
		bool		_registered;
		State		_state;
};

#endif
