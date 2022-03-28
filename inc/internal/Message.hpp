#pragma once

#include <data/Forward.hpp>
#include <iostream>
#include <internal/Origin.hpp>

#include <string>

// DM:		:ulambda!~clement@localhost PRIVMSG me :az
// Channel:	:ulambda!~clement@localhost PRIVMSG #test :az

namespace internal {
	class Message {
	private:
		Origin mOrigin;
		std::string mMessage;
		std::string mChannel;
		bool mNotice;

	public:
		Message(Origin origin, std::string message, bool notice = false);
		Message(Origin origin, std::string message, std::string channel, bool notice = false);
		Message(const Message &orig);
		~Message();

		Message &operator=(const Message &orig);

		Origin getOrigin() const;
		std::string getMessage() const;
		std::string getChannel() const;
		bool isNotice() const;

		bool hasChannel() const;

		bool trySetChannel(std::string channel);
	};

	std::ostream &operator<<(std::ostream &os, const Message &message);
} // namespace internal
