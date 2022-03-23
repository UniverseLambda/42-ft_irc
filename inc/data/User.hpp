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
			UMODE_END							= 0x10,
		};

		typedef UserMode Mode;

	private:
		int mFd;
		internal::ServerPtr mServer;

		std::string mSentPassword;

		std::string mNickname;
		std::string mUsername;
		std::string mRealname;
		std::string mHostname;
		bool mAuthenticated;

		UserMode mMode;

		std::set<ChannelPtr> mChannels;

	public:
		User();
		User(int fd, internal::ServerPtr server);
		User(const User &copy);
		~User();

		User &operator=(const User &rhs);

		void setSentPassword(const std::string &password);
		void setNickname(const std::string &nickname);
		void setUsername(const std::string &username);
		void setRealname(const std::string &realname);
		void setHostname(const std::string &hostname);
		void setAuthenticated(bool auth);
		void setMode(UserMode mode, bool addMode);

		int getFd() const;
		std::string getSentPassword() const;
		std::string getNickname() const;
		std::string getUsername() const;
		std::string getRealname() const;
		std::string getHostname() const;
		bool isAuthenticated() const;
		UserMode getMode() const;
		std::string getModeString() const;
		static char getModeChar(UserMode mode);
		static UserMode getMode(char c);
		bool isOperator() const;

		bool channelJoined(ChannelPtr channel);
		bool kickedFromChannel(ChannelPtr channel);

		bool sendMessage(internal::Message message);

		void dispatchDisconnect();
	};

	User::UserMode operator|(User::UserMode um0, User::UserMode um1);
	User::UserMode operator&(User::UserMode um0, User::UserMode um1);
	User::UserMode operator~(User::UserMode um);
}
