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
			CMODE_INVITE						= 0x002,	// i		SUPPORTED
			CMODE_TOPIC_OP_ONLY					= 0x004,	// t		SUPPORTED
			CMODE_BAN							= 0x008,	// b		SUPPORTED
			CMODE_END							= (CMODE_BAN << 1)
			// CMODE_PRIVATE					= 0x002,	// p		TO REMOVE
			// CMODE_SECRET						= 0x004,	// s		TO REMOVE
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
		void userDisconnected(UserPtr user, std::string message);

		void partMessage(UserPtr user, std::string name);
		void whoMessage(UserPtr user, std::string name);
		void namesMessage(UserPtr user);
		void inviteMessage(UserPtr user, std::string nickname, UserPtr target);
		void kickMessage(UserPtr user, std::vector<std::string> targets, std::string &comment);
		void answerList(UserPtr user) const;

		bool isInChannel(UserPtr user) const;

		bool sendMessage(UserPtr sender, internal::Message message);

		bool kickUser(UserPtr kicked);

		void userWillRename(UserPtr user, std::string newNick);
	};

	Channel::ChannelMode operator|(Channel::ChannelMode cm0, Channel::ChannelMode cm1);
	Channel::ChannelMode operator&(Channel::ChannelMode cm0, Channel::ChannelMode cm1);
	Channel::ChannelMode operator~(Channel::ChannelMode cm);
} // namespace data
