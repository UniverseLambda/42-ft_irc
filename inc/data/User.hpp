#pragma once

#include <string>
#include <set>

#include <data/Forward.hpp>

#include <internal/Forward.hpp>

#include <internal/Message.hpp>
#include <internal/Origin.hpp>

namespace data {
	class User {
	private:
		int mFd;
		internal::ServerPtr mServer;

		std::string mSentPassword;

		std::string mNickname;
		std::string mUsername;
		std::string mRealname;
		std::string mHostname;
		bool mAuthenticated;

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

		int getFd() const;
		std::string getSentPassword() const;
		std::string getNickname() const;
		std::string getUsername() const;
		std::string getRealname() const;
		std::string getHostname() const;
		bool isAuthenticated() const;
		internal::Origin getOrigin() const;

		bool channelJoined(ChannelPtr channel);
		bool kickedFromChannel(ChannelPtr channel);

		bool sendMessage(internal::Message message);

		void dispatchDisconnect(std::string message);
		void dispatchWillRename(std::string newNick);
	};
}
