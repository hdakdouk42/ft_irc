#include "Message.hpp"
#include "Utils.hpp"

Message	parseMessage(const std::string &line)
{
	Message							msg;
	std::string::size_type			i = 0;
	std::string::size_type			start;
	const std::string::size_type	len = line.size();

	while (i < len && line[i] == ' ')
		++i;

	if (i < len && line[i] == ':')
	{
		++i;
		start = i;
		while (i < len && line[i] != ' ')
			++i;
		msg.prefix = line.substr(start, i - start);
		while (i < len && line[i] == ' ')
			++i;
	}

	start = i;
	while (i < len && line[i] != ' ')
		++i;
	msg.command = utils::toUpper(line.substr(start, i - start));
	while (i < len && line[i] == ' ')
		++i;

	while (i < len)
	{
		if (line[i] == ':')
		{
			msg.params.push_back(line.substr(i + 1));
			break ;
		}
		start = i;
		while (i < len && line[i] != ' ')
			++i;
		msg.params.push_back(line.substr(start, i - start));
		while (i < len && line[i] == ' ')
			++i;
	}
	return (msg);
}
