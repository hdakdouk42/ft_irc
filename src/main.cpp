#include "Server.hpp"

#include <cctype>
#include <exception>
#include <iostream>
#include <string>

/*
** The port must be a plain number. Ports below 1024 are reserved and would
** need root privileges, so they are refused here.
*/
static bool	parsePort(const std::string &argument, unsigned short &port)
{
	unsigned long	value = 0;

	if (argument.empty() || argument.size() > 5)
		return (false);
	for (std::string::size_type i = 0; i < argument.size(); ++i)
	{
		if (!std::isdigit(static_cast<unsigned char>(argument[i])))
			return (false);
		value = value * 10 + static_cast<unsigned long>(argument[i] - '0');
	}
	if (value < 1024 || value > 65535)
		return (false);
	port = static_cast<unsigned short>(value);
	return (true);
}

int	main(int argc, char **argv)
{
	unsigned short	port = 0;

	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	if (!parsePort(argv[1], port))
	{
		std::cerr << "Error: <port> must be a number between 1024 and 65535"
			<< std::endl;
		return (1);
	}
	if (std::string(argv[2]).empty())
	{
		std::cerr << "Error: <password> must not be empty" << std::endl;
		return (1);
	}
	try
	{
		Server	server(port, std::string(argv[2]));

		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}
