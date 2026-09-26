#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <vector>

namespace utils
{
	std::vector<std::string>	split(const std::string &str, char separator);
	std::string					toUpper(const std::string &str);
	std::string					ircToLower(const std::string &str);
	bool						ircEqual(const std::string &a, const std::string &b);
	std::string					toString(long value);
	bool						parsePositiveNumber(const std::string &str, unsigned long &result);
	bool						isChannelName(const std::string &name);
	bool						isValidChannelName(const std::string &name);
	bool						isValidNickname(const std::string &nickname);
}

#endif
