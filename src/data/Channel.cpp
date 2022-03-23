#include <data/Channel.hpp>
#include <data/User.hpp>
#include <internal/Server.hpp>

#include <util/Util.hpp>

#include <sstream>

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

			if (!user->channelJoined(this)) {
				return false;
			}

			for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
				mServer->getCommInterface()->sendMessage(user->getFd(), internal::Origin(user->getNickname(), user->getUsername(), user->getHostname()), "JOIN", util::makeVector(mName), true);

				util::sendNumericReply(mServer->getCommInterface(), user, "353", util::makeVector(user->getNickname()));
			}

			return util::sendNumericReply(mServer->getCommInterface(), user, "366", util::makeVector<std::string>("End of /NAMES list"));
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

	std::string Channel::getModeString() const {
		std::ostringstream os;

		int v = 0x001;

		for (int i = 0; i != CMODE_END; i <<= 1) {
			os << getModeChar(static_cast<ChannelMode>(v));
		}

		return os.str();
	}

	char Channel::getModeChar(ChannelMode mode) {
		switch (mode) {
			case CMODE_OPERATOR:					return 'o';
			case CMODE_PRIVATE:						return 'p';
			case CMODE_SECRET:						return 's';
			case CMODE_INVITE:						return 'i';
			case CMODE_TOPIC_OP_ONLY:				return 't';
			case CMODE_NO_OUTSIDE_CLIENT:			return 'n';
			case CMODE_MODERATED:					return 'm';
			case CMODE_LIMIT:						return 'l';
			case CMODE_BAN:							return 'b';
			case CMODE_SPEAK_ON_MODERATED_CHANNEL:	return 'v';
			case CMODE_PASSWORD:					return 'k';
			default:								return '\0';
		}
	}

	Channel::ChannelMode Channel::getMode(char c) {
		switch (c) {
			case 'o':			return CMODE_OPERATOR;
			case 'p':			return CMODE_PRIVATE;
			case 's':			return CMODE_SECRET;
			case 'i':			return CMODE_INVITE;
			case 't':			return CMODE_TOPIC_OP_ONLY;
			case 'n':			return CMODE_NO_OUTSIDE_CLIENT;
			case 'm':			return CMODE_MODERATED;
			case 'l':			return CMODE_LIMIT;
			case 'b':			return CMODE_BAN;
			case 'v':			return CMODE_SPEAK_ON_MODERATED_CHANNEL;
			case 'k':			return CMODE_PASSWORD;
		}
		return CMODE_NONE;
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
