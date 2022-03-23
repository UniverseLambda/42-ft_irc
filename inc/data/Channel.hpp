#pragma once

#include <string>
#include <set>
#include <map>

#include <stdexcept>

#include "Forward.hpp"

#include <internal/Forward.hpp>
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
			CMODE_END							= 0x800
		};

		typedef ChannelMode Mode;

	private:
		typedef std::map<UserPtr, bool> user_storage;

		std::string mName;
		internal::ServerPtr mServer;
		user_storage mUsers;
		ChannelMode mMode;

	public:
		Channel();
		Channel(std::string name, internal::ServerPtr server);
		Channel(const Channel &orig);
		~Channel();

		Channel &operator=(const Channel &orig);

		std::string getName() const;

		void setOperator(UserPtr user, bool op) throw(std::out_of_range);
		bool isOperator(UserPtr user) const throw(std::out_of_range);

		bool setMode(ChannelMode mode, bool addMode);
		ChannelMode getMode() const;
		std::string getModeString() const;

		static char getModeChar(ChannelMode mode);
		static ChannelMode getMode(char c);

		bool userJoin(UserPtr user);
		void userDisconnected(UserPtr user);

		bool sendMessage(UserPtr sender, internal::Message message);

		bool kickUser(UserPtr kicked);
	};

	Channel::ChannelMode operator|(Channel::ChannelMode cm0, Channel::ChannelMode cm1);
	Channel::ChannelMode operator&(Channel::ChannelMode cm0, Channel::ChannelMode cm1);
	Channel::ChannelMode operator~(Channel::ChannelMode cm);
} // namespace data
