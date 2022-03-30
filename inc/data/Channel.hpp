#pragma once

#include <string>
#include <set>
#include <map>

#include <vector>

#include <stdexcept>

#include "Forward.hpp"

#include <internal/Forward.hpp>
#include <internal/Message.hpp>
#include <util/Optional.hpp>

namespace data {
	class Channel {
	public:
		enum ChannelMode {
															// Flags	Status
			CMODE_NONE							= 0x000,
			CMODE_OPERATOR						= 0x001,	// o		SUPPORTED
			CMODE_PRIVATE						= 0x002,	// p		TO REMOVE
			CMODE_SECRET						= 0x004,	// s		TO REMOVE
			CMODE_INVITE						= 0x008,	// i		SUPPORTED
			CMODE_TOPIC_OP_ONLY					= 0x010,	// t		TODO
			CMODE_BAN							= 0x020,	// b		SUPPORTED
			CMODE_END							= (CMODE_BAN << 1)
		};

		typedef ChannelMode Mode;

	private:
		typedef std::map<UserPtr, bool> user_storage;

		std::string mName;
		internal::ServerPtr mServer;
		user_storage mUsers;
		std::set<std::string> mBanList;
		std::set<std::string> mInviteList;
		std::string mTopic;
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

		void admitMode(UserPtr sender, std::string mode, bool addMode, std::vector<std::string> params);
		bool setMode(ChannelMode mode, bool addMode);
		ChannelMode getMode() const;
		std::string getModeString() const;

		static char getModeChar(ChannelMode mode);
		static ChannelMode getMode(char c);

		std::string getTopic() const;
		void setTopic(std::string &topic);
		void topicMessage(UserPtr user, util::Optional<std::string> topic = util::Optional<std::string>());

		bool userJoin(UserPtr user);
		void userDisconnected(UserPtr user);

		bool isInChannel(UserPtr user) const;

		bool sendMessage(UserPtr sender, internal::Message message);

		bool kickUser(UserPtr kicked);
	};

	Channel::ChannelMode operator|(Channel::ChannelMode cm0, Channel::ChannelMode cm1);
	Channel::ChannelMode operator&(Channel::ChannelMode cm0, Channel::ChannelMode cm1);
	Channel::ChannelMode operator~(Channel::ChannelMode cm);
} // namespace data
