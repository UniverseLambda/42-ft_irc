#pragma once

#include <string>
#include <set>
#include <map>

#include <stdexcept>

#include "Forward.hpp"

#include <internal/Message.hpp>

namespace data {
	class Channel {
	public:
		enum ChannelMode {
															// Flags
			CMODE_NONE							= 0x000,
			CMODE_OPERATOR						= 0x001,	// o
			CMODE_PRIVATE						= 0x002,	// p
			CMODE_SECRET						= 0x004,	// s
			CMODE_INVITE						= 0x008,	// i
			CMODE_TOPIC_OP_ONLY					= 0x010,	// t
			CMODE_NO_OUTSIDE_CLIENT				= 0x020,	// n
			CMODE_MODERATED						= 0x040,	// m
			CMODE_LIMIT							= 0x080,	// l
			CMODE_BAN							= 0x100,	// b
			CMODE_SPEAK_ON_MODERATED_CHANNEL	= 0x200,	// v
			CMODE_PASSWORD						= 0x400,	// k
		};

		enum UserMode {
			UMODE_NONE							= 0x00,
			UMODE_INVISIBLE						= 0x01,
			UMODE_NOTICE_RECEIPT				= 0x02,
			UMODE_WALLOPS_RECEIVER				= 0x04,
			UMODE_OPERATOR						= 0x08,
		};

	private:
		std::string mName;
		std::map<UserPtr, UserMode> mUsers;
		ChannelMode mMode;

	public:
		Channel(std::string name);
		Channel(const Channel &orig);
		~Channel();

		Channel &operator=(const Channel &orig);

		std::string getName() const;

		bool isOperator(UserPtr user) const;

		bool userJoin(UserPtr user);
		void userDisconnected(UserPtr user);

		bool setUserMode(UserPtr user, UserMode mode, bool addMode);
		bool setChannelMode(ChannelMode mode, bool addMode);

		UserMode getUserMode(UserPtr user) const throw(std::out_of_range);
		ChannelMode getChannelMode() const;

		bool sendMessage(UserPtr sender, internal::Message message);
	};

	Channel::ChannelMode operator|(Channel::ChannelMode cm0, Channel::ChannelMode cm1);
	Channel::ChannelMode operator&(Channel::ChannelMode cm0, Channel::ChannelMode cm1);
	Channel::ChannelMode operator~(Channel::ChannelMode cm);

	Channel::UserMode operator|(Channel::UserMode cm0, Channel::UserMode cm1);
	Channel::UserMode operator&(Channel::UserMode cm0, Channel::UserMode cm1);
	Channel::UserMode operator~(Channel::UserMode um);
} // namespace data
