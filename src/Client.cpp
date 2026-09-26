#include "Client.hpp"

Client::Client(int fd, const std::string &host)
	: _fd(fd),
	  _host(host),
	  _nickname(""),
	  _username(""),
	  _realname(""),
	  _in(""),
	  _out(""),
	  _hasPassword(false),
	  _hasUserInfo(false),
	  _registered(false),
	  _state(CONNECTED)
{
}

Client::~Client()
{
}

int	Client::getFd() const
{
	return (_fd);
}

const std::string	&Client::getHost() const
{
	return (_host);
}

const std::string	&Client::getNickname() const
{
	return (_nickname);
}

const std::string	&Client::getUsername() const
{
	return (_username);
}

const std::string	&Client::getRealname() const
{
	return (_realname);
}

std::string	Client::getNickOrStar() const
{
	if (_nickname.empty())
		return ("*");
	return (_nickname);
}

std::string	Client::getPrefix() const
{
	return (getNickOrStar() + "!" + (_username.empty() ? "*" : _username)
		+ "@" + _host);
}

void	Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}

void	Client::setUser(const std::string &username, const std::string &realname)
{
	_username = username;
	_realname = realname;
	_hasUserInfo = true;
}

bool	Client::hasPassword() const
{
	return (_hasPassword);
}

void	Client::setPassword(bool value)
{
	_hasPassword = value;
}

bool	Client::hasUserInfo() const
{
	return (_hasUserInfo);
}

bool	Client::isRegistered() const
{
	return (_registered);
}

void	Client::setRegistered(bool value)
{
	_registered = value;
}

Client::State	Client::getState() const
{
	return (_state);
}

void	Client::setState(Client::State state)
{
	_state = state;
}

bool	Client::isClosing() const
{
	return (_state == CLOSING);
}

bool	Client::isDead() const
{
	return (_state == DEAD);
}

bool	Client::isAlive() const
{
	return (_state == CONNECTED);
}

std::string	&Client::inBuffer()
{
	return (_in);
}

std::string	&Client::outBuffer()
{
	return (_out);
}

void	Client::appendOut(const std::string &data)
{
	_out += data;
}
