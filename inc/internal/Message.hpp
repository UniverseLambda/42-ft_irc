#pragma once

#include <data/Forward.hpp>
#include <iostream>

#include <string>

// DM:		:ulambda!~clement@localhost PRIVMSG me :az
// Channel:	:ulambda!~clement@localhost PRIVMSG #test :az

namespace internal {
	class Message {
	private:
		std::string mOrigin;
		std::string mMessage;
		std::string mChannel;

	public:
		Message(std::string origin, std::string message);
		Message(std::string origin, std::string message, std::string channel);
		Message(const Message &orig);
		~Message();

		Message &operator=(const Message &orig);

		std::string getOrigin() const;
		std::string getMessage() const;
		std::string getChannel() const;

		bool hasChannel() const;

		bool trySetChannel(std::string channel);
	};

	std::ostream &operator<<(std::ostream &os, const Message &message);
} // namespace internal
