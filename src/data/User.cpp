#include <data/User.hpp>
#include <data/Channel.hpp>

#include <sstream>

#include <api/IComm.hpp>

#include <internal/Server.hpp>

#include <util/Util.hpp>

namespace data {
	User::User(): mFd(-1), mServer(NULL), mAuthenticated(false) {}

	User::User(int fd, internal::ServerPtr server): mFd(fd), mServer(server), mAuthenticated(false) {}

	User::User(const User &copy):
		mFd(copy.mFd),
		mServer(copy.mServer),
		mNickname(copy.mNickname),
		mUsername(copy.mUsername),
		mRealname(copy.mRealname),
		mHostname(copy.mHostname),
		mAuthenticated(copy.mAuthenticated) {}

	User::~User() {}

	User &User::operator=(const User &rhs) {
		mFd = rhs.mFd;
		mServer = rhs.mServer;
		mNickname = rhs.mNickname;
		mUsername = rhs.mUsername;
		mRealname = rhs.mRealname;
		mHostname = rhs.mHostname;
		mAuthenticated = rhs.mAuthenticated;

		return *this;
	}

	void User::setSentPassword(const std::string &password) {
		mSentPassword = password;
	}

	void User::setNickname(const std::string &nickname) {
		mNickname = nickname;
	}

	void User::setUsername(const std::string &username) {
		mUsername = /* "~" +*/ username;
	}

	void User::setRealname(const std::string &realname) {
		mRealname = realname;
	}

	void User::setHostname(const std::string &hostname) {
		mHostname = hostname;
	}

	void User::setAuthenticated(bool authenticated) {
		mAuthenticated = authenticated;
	}

	int User::getFd() const {
		return mFd;
	}

	std::string User::getSentPassword() const {
		return mSentPassword;
	}

	std::string User::getNickname() const {
		return mNickname;
	}

	std::string User::getUsername() const {
		return mUsername;
	}

	std::string User::getRealname() const {
		return mRealname;
	}

	std::string User::getHostname() const {
		return mHostname;
	}

	bool User::isAuthenticated() const {
		return mAuthenticated;
	}

	internal::Origin User::getOrigin() const {
		return internal::Origin(mNickname, mUsername, mHostname);
	}

	bool User::channelJoined(ChannelPtr channel) {
		return mChannels.insert(channel).second;
	}

	bool User::kickedFromChannel(ChannelPtr channel) {
		return mChannels.erase(channel) != 0;
	}

	bool User::sendMessage(internal::Message message) {
		message.trySetChannel(mNickname);
		return mServer->sendMessage(this, message.getOrigin(),
			(message.isNotice() ? "NOTICE" : "PRIVMSG"),
			util::makeVector(message.getChannel(), message.getMessage()), true);
	}

	void User::dispatchDisconnect(std::string message) {
		for (std::set<ChannelPtr>::iterator it = mChannels.begin(); it != mChannels.end(); ++it) {
			(*it)->userDisconnected(this, message);
		}
	}

	void User::dispatchWillRename(std::string newNick) {
		for (std::set<ChannelPtr>::iterator it = mChannels.begin(); it != mChannels.end(); ++it) {
			(*it)->userWillRename(this, newNick);
		}

		mServer->sendMessage(this, getOrigin(), "NICK", newNick);
		mNickname = newNick;

	}
} // namespace data
