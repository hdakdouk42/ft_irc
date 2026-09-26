#ifndef REPLIES_HPP
# define REPLIES_HPP

# include <string>

# define SERVER_NAME		"ircserv"
# define SERVER_VERSION		"1.0"
# define CHANNEL_MODES		"itkol"

# define RPL_WELCOME(nick, prefix)	(":" SERVER_NAME " 001 " + (nick) + " :Welcome to the ft_irc Network, " + (prefix) + "\r\n")

# define RPL_YOURHOST(nick)			(":" SERVER_NAME " 002 " + (nick) + " :Your host is " SERVER_NAME ", running version " SERVER_VERSION "\r\n")

# define RPL_CREATED(nick, date)	(":" SERVER_NAME " 003 " + (nick) + " :This server was created " + (date) + "\r\n")

# define RPL_MYINFO(nick)			(":" SERVER_NAME " 004 " + (nick) + " " SERVER_NAME " " SERVER_VERSION " o " CHANNEL_MODES "\r\n")

# define RPL_ISUPPORT(nick)			(":" SERVER_NAME " 005 " + (nick) + " CHANTYPES=#& PREFIX=(o)@ CHANMODES=,k,l,it CASEMAPPING=rfc1459 :are supported by this server\r\n")

# define RPL_UMODEIS(nick)			(":" SERVER_NAME " 221 " + (nick) + " +\r\n")

# define RPL_CHANNELMODEIS(nick, channel, modes) \
									(":" SERVER_NAME " 324 " + (nick) + " " + (channel) + " " + (modes) + "\r\n")

# define RPL_NOTOPIC(nick, channel)	(":" SERVER_NAME " 331 " + (nick) + " " + (channel) + " :No topic is set\r\n")

# define RPL_TOPIC(nick, channel, topic) \
									(":" SERVER_NAME " 332 " + (nick) + " " + (channel) + " :" + (topic) + "\r\n")

# define RPL_INVITING(nick, target, channel) \
									(":" SERVER_NAME " 341 " + (nick) + " " + (target) + " " + (channel) + "\r\n")

# define RPL_NAMREPLY(nick, channel, names) \
									(":" SERVER_NAME " 353 " + (nick) + " = " + (channel) + " :" + (names) + "\r\n")

# define RPL_ENDOFNAMES(nick, channel) \
									(":" SERVER_NAME " 366 " + (nick) + " " + (channel) + " :End of /NAMES list\r\n")

# define ERR_NOSUCHNICK(nick, target) \
									(":" SERVER_NAME " 401 " + (nick) + " " + (target) + " :No such nick/channel\r\n")

# define ERR_NOSUCHCHANNEL(nick, channel) \
									(":" SERVER_NAME " 403 " + (nick) + " " + (channel) + " :No such channel\r\n")

# define ERR_CANNOTSENDTOCHAN(nick, channel) \
									(":" SERVER_NAME " 404 " + (nick) + " " + (channel) + " :Cannot send to channel\r\n")

# define ERR_NOORIGIN(nick)			(":" SERVER_NAME " 409 " + (nick) + " :No origin specified\r\n")

# define ERR_NORECIPIENT(nick, command) \
									(":" SERVER_NAME " 411 " + (nick) + " :No recipient given (" + (command) + ")\r\n")

# define ERR_NOTEXTTOSEND(nick)		(":" SERVER_NAME " 412 " + (nick) + " :No text to send\r\n")

# define ERR_UNKNOWNCOMMAND(nick, command) \
									(":" SERVER_NAME " 421 " + (nick) + " " + (command) + " :Unknown command\r\n")

# define ERR_NOMOTD(nick)			(":" SERVER_NAME " 422 " + (nick) + " :MOTD File is missing\r\n")

# define ERR_NONICKNAMEGIVEN(nick)	(":" SERVER_NAME " 431 " + (nick) + " :No nickname given\r\n")

# define ERR_ERRONEUSNICKNAME(nick, bad) \
									(":" SERVER_NAME " 432 " + (nick) + " " + (bad) + " :Erroneous nickname\r\n")

# define ERR_NICKNAMEINUSE(nick, bad) \
									(":" SERVER_NAME " 433 " + (nick) + " " + (bad) + " :Nickname is already in use\r\n")

# define ERR_USERNOTINCHANNEL(nick, target, channel) \
									(":" SERVER_NAME " 441 " + (nick) + " " + (target) + " " + (channel) + " :They aren't on that channel\r\n")

# define ERR_NOTONCHANNEL(nick, channel) \
									(":" SERVER_NAME " 442 " + (nick) + " " + (channel) + " :You're not on that channel\r\n")

# define ERR_USERONCHANNEL(nick, target, channel) \
									(":" SERVER_NAME " 443 " + (nick) + " " + (target) + " " + (channel) + " :is already on channel\r\n")

# define ERR_NOTREGISTERED(nick)	(":" SERVER_NAME " 451 " + (nick) + " :You have not registered\r\n")

# define ERR_NEEDMOREPARAMS(nick, command) \
									(":" SERVER_NAME " 461 " + (nick) + " " + (command) + " :Not enough parameters\r\n")

# define ERR_ALREADYREGISTRED(nick)	(":" SERVER_NAME " 462 " + (nick) + " :You already reregister\r\n")

# define ERR_PASSWDMISMATCH(nick)	(":" SERVER_NAME " 464 " + (nick) + " :Password incorrect\r\n")

# define ERR_CHANNELISFULL(nick, channel) \
									(":" SERVER_NAME " 471 " + (nick) + " " + (channel) + " :Cannot join channel (+l)\r\n")

# define ERR_UNKNOWNMODE(nick, mode)(":" SERVER_NAME " 472 " + (nick) + " " + (mode) + " :is unknown mode char to me\r\n")

# define ERR_INVITEONLYCHAN(nick, channel) \
									(":" SERVER_NAME " 473 " + (nick) + " " + (channel) + " :Cannot join channel (+i)\r\n")

# define ERR_BADCHANNELKEY(nick, channel) \
									(":" SERVER_NAME " 475 " + (nick) + " " + (channel) + " :Cannot join channel (+k)\r\n")

# define ERR_CHANOPRIVSNEEDED(nick, channel) \
									(":" SERVER_NAME " 482 " + (nick) + " " + (channel) + " :You're not channel operator\r\n")


# define ERR_USERSDONTMATCH(nick) \
									(":" SERVER_NAME " 502 " + (nick) + " :Cannot change mode for other users\r\n")

#endif
