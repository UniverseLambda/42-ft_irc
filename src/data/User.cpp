#include <data/User.hpp>
#include <data/Channel.hpp>

#include <api/IComm.hpp>

#include <internal/Server.hpp>

#include <util/Util.hpp>

namespace data {
	User::User(): mFd(-1), mServer(NULL), mAuthenticated(false), mMode(UMODE_NONE) {}

	User::User(int fd, internal::ServerPtr server): mFd(fd), mServer(server), mAuthenticated(false), mMode(UMODE_NONE) {}

	User::User(const User &copy):
		mFd(copy.mFd),
		mServer(copy.mServer),
		mNickname(copy.mNickname),
		mUsername(copy.mUsername),
		mRealname(copy.mRealname),
		mHostname(copy.mHostname),
		mAuthenticated(copy.mAuthenticated),
		mMode(copy.mMode) {

	}

	User::~User() {}

	User &User::operator=(const User &rhs) {
		mFd = rhs.mFd;
		mServer = rhs.mServer;
		mNickname = rhs.mNickname;
		mUsername = rhs.mUsername;
		mRealname = rhs.mRealname;
		mHostname = rhs.mHostname;
		mAuthenticated = rhs.mAuthenticated;
		mMode = rhs.mMode;

		return *this;
	}

	void User::setNickname(const std::string &nickname) {
		mNickname = nickname;
	}

	void User::setUsername(const std::string &username) {
		mUsername = username;
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

	void User::setMode(UserMode mode, bool addMode) {
		if (addMode) {
			mMode = mMode | mode;
		} else {
			mMode = mMode & (~mode);
		}
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

	bool User::getAuthenticated() const {
		return mAuthenticated;
	}

	User::UserMode User::getMode() const {
		return mMode;
	}

	bool User::isOperator() const {
		return !!(mMode & UMODE_OPERATOR);
	}

	bool User::channelJoined(ChannelPtr channel) {
		return mChannels.insert(channel).second;
	}

	bool User::kickedFromChannel(ChannelPtr channel) {
		return mChannels.erase(channel) != 0;
	}

	bool User::sendMessage(internal::Message message) {
		message.trySetChannel(mNickname);
		return mServer->getCommInterface()->sendMessage(mFd, message.getOrigin(), "PRIVMSG", util::makeVector(message.getChannel(), message.getMessage()), true);
	}

	void User::dispatchDisconnect() {
		for (std::set<ChannelPtr>::iterator it = mChannels.begin(); it != mChannels.end(); ++it) {
			(*it)->userDisconnected(this);
		}
	}

	User::UserMode operator|(User::UserMode um0, User::UserMode um1) {
		return static_cast<User::UserMode>(static_cast<int>(um0) | static_cast<int>(um1));
	}

	User::UserMode operator&(User::UserMode um0, User::UserMode um1) {
		return static_cast<User::UserMode>(static_cast<int>(um0) & static_cast<int>(um1));
	}

	User::UserMode operator~(User::UserMode um) {
		return static_cast<User::UserMode>(~(static_cast<int>(um)));
	}
} // namespace data
