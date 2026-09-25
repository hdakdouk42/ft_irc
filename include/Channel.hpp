#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <cstddef>
# include <string>
# include <vector>

class Client;

class Channel
{
	private:
		Channel(const Channel &other);
		Channel	&operator=(const Channel &other);

		std::string				_name;
		std::string				_topic;
		std::vector<Client *>	_members;
		std::vector<Client *>	_operators;
		std::vector<Client *>	_invited;
		bool					_inviteOnly;
		bool					_topicRestricted;
		bool					_hasKey;
		std::string				_key;
		bool					_hasUserLimit;
		std::size_t				_userLimit;


	public:
		explicit Channel(const std::string &name);
		~Channel();

		const std::string	&getName() const;
		const std::string	&getTopic() const;
		void				setTopic(const std::string &topic);

		void				addMember(Client *client);
		Client				*removeMember(Client *client);
		bool				isMember(Client *client) const;
		bool				isEmpty() const;
		std::size_t			size() const;
		const std::vector<Client *>	&getMembers() const;

		void				addOperator(Client *client);
		void				removeOperator(Client *client);

		Client				*promoteIfNoOperator(Client *except);
		bool				isOperator(Client *client) const;

		void				addInvite(Client *client);
		void				removeInvite(Client *client);
		bool				isInvited(Client *client) const;

		bool				isInviteOnly() const;
		void				setInviteOnly(bool value);

		bool				isTopicRestricted() const;
		void				setTopicRestricted(bool value);

		bool				hasKey() const;
		const std::string	&getKey() const;
		void				setKey(const std::string &key);
		void				removeKey();

		bool				hasUserLimit() const;
		std::size_t			getUserLimit() const;
		void				setUserLimit(std::size_t limit);
		void				removeUserLimit();

		std::string			getModeString(bool withArguments) const;

		std::string			getNamesList() const;
};

#endif
