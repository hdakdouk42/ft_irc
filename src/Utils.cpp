#include "Utils.hpp"

#include <cctype>
#include <sstream>

namespace utils {

static const char	*g_nickSpecial = "[]\\`_^{|}";

static const std::string::size_type	g_maxNickLength = 30;
static const std::string::size_type	g_maxChannelLength = 50;

std::vector<std::string>	split(const std::string &str, char separator)
{
	std::vector<std::string>	result;
	std::string::size_type		start = 0;
	std::string::size_type		pos;

	while ((pos = str.find(separator, start)) != std::string::npos)
	{
		result.push_back(str.substr(start, pos - start));
		start = pos + 1;
	}
	result.push_back(str.substr(start));
	return (result);
}

std::string	toUpper(const std::string &str)
{
	std::string	result(str);

	for (std::string::size_type i = 0; i < result.size(); ++i)
		result[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[i])));
	return (result);
}

std::string	ircToLower(const std::string &str)
{
	std::string	result(str);

	for (std::string::size_type i = 0; i < result.size(); ++i)
	{
		char	c = result[i];

		if (c >= 'A' && c <= 'Z')
			c = static_cast<char>(c - 'A' + 'a');
		else if (c == '[')
			c = '{';
		else if (c == ']')
			c = '}';
		else if (c == '\\')
			c = '|';
		else if (c == '~')
			c = '^';
		result[i] = c;
	}
	return (result);
}

bool	ircEqual(const std::string &a, const std::string &b)
{
	return (ircToLower(a) == ircToLower(b));
}

std::string	toString(long value)
{
	std::ostringstream	stream;

	stream << value;
	return (stream.str());
}

bool	parsePositiveNumber(const std::string &str, unsigned long &result)
{
	unsigned long	value = 0;

	if (str.empty() || str.size() > 9)
		return (false);
	for (std::string::size_type i = 0; i < str.size(); ++i)
	{
		if (!std::isdigit(static_cast<unsigned char>(str[i])))
			return (false);
		value = value * 10 + static_cast<unsigned long>(str[i] - '0');
	}
	if (value == 0)
		return (false);
	result = value;
	return (true);
}

bool	isChannelName(const std::string &name)
{
	return (!name.empty() && (name[0] == '#' || name[0] == '&'));
}

bool	isValidChannelName(const std::string &name)
{
	if (name.size() < 2 || name.size() > g_maxChannelLength)
		return (false);
	if (!isChannelName(name))
		return (false);
	for (std::string::size_type i = 1; i < name.size(); ++i)
	{
		const char	c = name[i];

		if (c == ' ' || c == ',' || c == ':' || c == 7 || c == '\r' || c == '\n')
			return (false);
	}
	return (true);
}

bool	isValidNickname(const std::string &nickname)
{
	if (nickname.empty() || nickname.size() > g_maxNickLength)
		return (false);
	if (!std::isalpha(static_cast<unsigned char>(nickname[0])) && std::string(g_nickSpecial).find(nickname[0]) == std::string::npos)
		return (false);
	for (std::string::size_type i = 1; i < nickname.size(); ++i)
	{
		const char	c = nickname[i];

		if (std::isalnum(static_cast<unsigned char>(c)))
			continue ;
		if (c == '-')
			continue ;
		if (std::string(g_nickSpecial).find(c) != std::string::npos)
			continue ;
		return (false);
	}
	return (true);
}
}
