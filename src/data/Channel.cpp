#include <data/Channel.hpp>
#include <data/User.hpp>

namespace data {
	Channel::Channel():
		mServer(NULL),
		mMode(CMODE_NONE) {}

	Channel::Channel(std::string name, internal::ServerPtr server):
		mName(name), mServer(server), mMode(CMODE_NONE) {}

	Channel::Channel(const Channel &orig):
		mName(orig.mName),
		mServer(orig.mServer),
		mUsers(orig.mUsers),
		mMode(orig.mMode) {}

	Channel::~Channel() {}

	Channel &Channel::operator=(const Channel &orig) {
		mName = orig.mName;
		mServer = orig.mServer;
		mUsers = orig.mUsers;
		return *this;
	}

	std::string Channel::getName() const {
		return mName;
	}

	void Channel::setOperator(UserPtr user, bool op) throw(std::out_of_range) {
		mUsers.at(user) = op;
	}

	bool Channel::isOperator(UserPtr user) const throw(std::out_of_range) {
		return mUsers.at(user);
	}

	bool Channel::userJoin(UserPtr user) {
		try {
			if (!mUsers.insert(std::make_pair(user, mUsers.empty())).second) {
				return false;
			}

			return user->channelJoined(this);
		} catch (...) {}
		return false;
	}

	void Channel::userDisconnected(UserPtr user) {
		try {
			mUsers.erase(user);
		} catch (...) {}
	}

	bool Channel::setMode(ChannelMode mode, bool addMode) {
		if (addMode) {
			mMode = mMode | mode;
		} else {
			mMode = mMode & (~mode);
		}
		return true;
	}

	Channel::ChannelMode Channel::getMode() const {
		return mMode;
	}

	bool Channel::sendMessage(UserPtr sender, internal::Message message) {
		if (mUsers.count(sender) == 0) {
			return false;
		}

		message.trySetChannel(mName);

		for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
			if (it->first != sender) {
				it->first->sendMessage(message);
			}
		}

		return true;
	}

	bool Channel::kickUser(UserPtr kicked) {
		return !!(mUsers.erase(kicked)) && kicked->kickedFromChannel(this);
	}

	Channel::ChannelMode operator|(Channel::ChannelMode cm0, Channel::ChannelMode cm1) {
		return static_cast<Channel::ChannelMode>(static_cast<int>(cm0) | static_cast<int>(cm1));
	}

	Channel::ChannelMode operator&(Channel::ChannelMode cm0, Channel::ChannelMode cm1) {
		return static_cast<Channel::ChannelMode>(static_cast<int>(cm0) & static_cast<int>(cm1));
	}

	Channel::ChannelMode operator~(Channel::ChannelMode cm) {
		return static_cast<Channel::ChannelMode>(~(static_cast<int>(cm)));
	}
} // namespace data
