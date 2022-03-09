#pragma once

#include <string>
#include <set>

#include <data/Forward.hpp>

#include <internal/Forward.hpp>

#include <internal/Message.hpp>

namespace data {
	class User {
	public:
		enum UserMode {
			UMODE_NONE							= 0x00,
			UMODE_INVISIBLE						= 0x01,
			UMODE_NOTICE_RECEIPT				= 0x02,
			UMODE_WALLOPS_RECEIVER				= 0x04,
			UMODE_OPERATOR						= 0x08,
		};

	private:
		int mFd;
		internal::ServerPtr mServer;

		std::string mNickname;
		std::string mUsername;
		std::string mRealname;
		bool mAuthenticated;

		UserMode mMode;

		std::set<ChannelPtr> mChannels;

	public:
		User();
		User(int fd, internal::ServerPtr server);
		User(const User &copy);
		~User();

		User &operator=(const User &rhs);

		void setNickname(const std::string &nickname);
		void setUsername(const std::string &username);
		void setRealname(const std::string &realname);
		void setAuthenticated(bool auth);
		void setMode(UserMode mode, bool addMode);

		std::string getNickname() const;
		std::string getUsername() const;
		std::string getRealname() const;
		bool getAuthenticated() const;
		UserMode getMode() const;
		bool isOperator() const;

		bool channelDestroyed(ChannelPtr channel);

		bool sendMessage(internal::Message message);
	};

	User::UserMode operator|(User::UserMode um0, User::UserMode um1);
	User::UserMode operator&(User::UserMode um0, User::UserMode um1);
	User::UserMode operator~(User::UserMode um);
}
