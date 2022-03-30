#include <data/Channel.hpp>
#include <data/User.hpp>
#include <internal/Server.hpp>

#include <util/Util.hpp>

#include <algorithm>
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
		mBanList(orig.mBanList),
		mInviteList(orig.mInviteList),
		mTopic(orig.mTopic),
		mMode(orig.mMode) {}

	Channel::~Channel() {}

	Channel &Channel::operator=(const Channel &orig) {
		mName = orig.mName;
		mServer = orig.mServer;
		mUsers = orig.mUsers;
		mBanList = orig.mBanList;
		mInviteList = orig.mInviteList;
		mTopic = orig.mTopic;
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
			if (mBanList.count(user->getNickname()) == 1) {
				mServer->sendNumericReply(user, "474", util::makeVector<std::string>(mName, "Cannot join channel (+b)"));
				return false;
			}

			if (mMode & CMODE_INVITE) {
				std::set<std::string>::iterator res = mInviteList.find(user->getNickname());

				if (res == mInviteList.end()) {
					mServer->sendNumericReply(user, "473", util::makeVector<std::string>(mName, "Cannot join channel (+i)"));
					return false;
				}
			}
			// TODO: Check for invite

			if (!mUsers.insert(std::make_pair(user, mUsers.empty())).second) {
				return false;
			}

			if (mMode & CMODE_INVITE) {
				mInviteList.erase(user->getNickname());
			}

			if (!user->channelJoined(this)) {
				return false;
			}

			for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
				mServer->sendMessage(it->first, user->getOrigin(), "JOIN", mName, true);

				mServer->sendNumericReply(user, "353",
					util::makeVector<std::string>(
						(mMode & CMODE_PRIVATE) ? "*" : (mMode & CMODE_SECRET) ? "@" : "=",
						mName,
						(it->second ? "@" : "") + it->first->getNickname()
				));
			}

			return mServer->sendNumericReply(user, "366", "End of NAMES list");
		} catch (...) {}
		return false;
	}

	void Channel::userDisconnected(UserPtr user) {
		try {
			mUsers.erase(user);
		} catch (...) {}
	}

	bool Channel::isInChannel(UserPtr user) const {
		return !!mUsers.count(user);
	}

	void Channel::admitMode(UserPtr sender, std::string modes, bool addMode, std::vector<std::string> params) {
		std::vector<UserPtr> newOps;
		std::vector<std::string> newBans;
		Mode finalMode;

		for (std::size_t i = 0, p = 0; i < modes.size(); ++i) {
			Mode mode = getMode(modes[i]);

			if (!mode) {
				mServer->sendNumericReply(sender, "472", util::makeVector<std::string>(modes.substr(i, 1), "is unknown mode char to me for " + mName));
				return;
			} else if (mode == CMODE_OPERATOR) {
				if (p + 1 >= params.size()) {
					mServer->sendNumericReply(sender, "461", util::makeVector<std::string>("MODE", "Not enough parameters"));
					return;
				}

				std::string &user = params[p++];

				UserPtr userPtr = mServer->getUser(user);

				if (!userPtr || mUsers.count(userPtr) == 0) {
					mServer->sendNumericReply(sender, "441", util::makeVector<std::string>(user, mName, "They aren't on that channel"));
					return;
				}

				newOps.push_back(userPtr);
			} else if (mode == CMODE_BAN) {
				if (p + 1 >= params.size()) {
					mServer->sendNumericReply(sender, "461", util::makeVector<std::string>("MODE", "Not enough parameters"));
					return;
				}

				newBans.push_back(params[p++]);
			} else {
				finalMode = finalMode | mode;
			}
		}

		for (std::vector<UserPtr>::iterator it = newOps.begin(); it != newOps.end(); ++it) {
			mUsers[*it] = addMode;
		}


		for (std::vector<std::string>::iterator it = newBans.begin(); it != newBans.end(); ++it) {
			if (addMode) {
				mBanList.insert(*it);
			} else {
				mBanList.erase(*it);
			}
		}

		for (std::map<UserPtr, bool>::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
			mServer->sendMessage(it->first, it->first->getOrigin(), "MODE", util::makeVector<std::string>(mName, (addMode ? "+" : "-") + modes));
		}
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

		for (int i = 0x001; i != CMODE_END; i <<= 1) {
			os << getModeChar(static_cast<ChannelMode>(i));
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
			case CMODE_BAN:							return 'b';
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
			case 'b':			return CMODE_BAN;
		}
		return CMODE_NONE;
	}

	std::string Channel::getTopic() const {
		return mTopic;
	}

	void Channel::setTopic(std::string &topic) {
		mTopic = topic;
	}

	void Channel::topicMessage(UserPtr user, util::Optional<std::string> topic) {
		user_storage::iterator uit = mUsers.find(user);

		if (uit == mUsers.end()) {
			mServer->sendNumericReply(user, "442", util::makeVector<std::string>(mName, "You're not on that channel"));
			return;
		}

		if (topic) {
			if ((mMode & CMODE_TOPIC_OP_ONLY) && !(uit->second)) {
				mServer->sendNumericReply(user, "482", util::makeVector<std::string>(mName, "You're not channel operator"));
			} else {
				mTopic = *topic;
				for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
					mServer->sendMessage(it->first, user->getOrigin(), "TOPIC", util::makeVector(mName, mTopic), true);
				}
			}
		} else {
			if (mTopic.empty()) {
				mServer->sendNumericReply(user, "331", util::makeVector<std::string>(mName, "No topic is set"));
			} else {
				mServer->sendNumericReply(user, "332", util::makeVector<std::string>(mName, mTopic));
			}
		}
	}

	// :irc.example.net 352 nick #test ~clsaad localhost irc.example.net nick H@ :0 realname
	void Channel::whoMessage(UserPtr user, std::string name) {
		for (user_storage::iterator uit = mUsers.begin(); uit != mUsers.end(); ++uit) {
			mServer->sendNumericReply(user, "352", util::makeVector<std::string>(
				mName,
				uit->first->getUsername(),
				uit->first->getHostname(),
				mServer->getHost(),
				uit->first->getNickname(),
				std::string() + "H" + ((mMode & CMODE_PRIVATE) ? "*" : "") + (uit->second ? "@" : ""),
				"0 " + uit->first->getRealname()
				));
		}

		mServer->sendNumericReply(user, "315", util::makeVector<std::string>(name, "End of WHO list"));
	}

	void Channel::namesMessage(UserPtr user) {
		for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
			mServer->sendMessage(it->first, user->getOrigin(), "JOIN", mName, true);

			mServer->sendNumericReply(user, "353",
				util::makeVector<std::string>(
					(mMode & CMODE_PRIVATE) ? "*" : (mMode & CMODE_SECRET) ? "@" : "=",
					mName,
					(it->second ? "@" : "") + it->first->getNickname()
			));
		}

		mServer->sendNumericReply(user, "366", "End of NAMES list");
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
