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

			if (!mUsers.insert(std::make_pair(user, mUsers.empty())).second) {
				return false;
			}

			if (mMode & CMODE_INVITE) {
				mInviteList.erase(user->getNickname());
			}

			if (!user->channelJoined(this)) {
				return false;
			}

			mServer->sendMessage(user, user->getOrigin(), "JOIN", mName, true);
			if (mTopic.empty()) {
				mServer->sendNumericReply(user, "331", util::makeVector<std::string>(mName, "No topic is set"));
			} else {
				mServer->sendNumericReply(user, "332", util::makeVector(mName, mTopic));
			}

			for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
				if (it->first != user) {
					mServer->sendMessage(it->first, user->getOrigin(), "JOIN", mName, true);
				}

				mServer->sendNumericReply(user, "353",
					util::makeVector<std::string>(
						/*(mMode & CMODE_PRIVATE) ? "*" : (mMode & CMODE_SECRET) ? "@" : */"=",
						mName,
						(it->second ? "@" : "") + it->first->getNickname()
				));
			}

			return mServer->sendNumericReply(user, "366", util::makeVector<std::string>(mName, "End of NAMES list"));
		} catch (...) {}
		return false;
	}

	void Channel::userDisconnected(UserPtr user, std::string message) {
		try {
			mUsers.erase(user);
		} catch (...) {}

		for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
			mServer->sendMessage(it->first, user->getOrigin(), "QUIT", message, true);
		}


		if (mUsers.empty()) {
			mServer->channelReclaiming(mName);
			delete this;
		}
	}

	bool Channel::isInChannel(UserPtr user) const {
		return !!mUsers.count(user);
	}

	void Channel::admitMode(UserPtr sender, std::string modes, bool addMode, std::vector<std::string> params) {
		std::vector<UserPtr> newOps;
		std::vector<std::string> newBans;
		Mode finalMode;

		std::cout << "PARAMS1" << std::endl;
		for (std::size_t i = 0; i < params.size(); ++i) {
			std::cout << "- " << params[i] << std::endl;
		}

		for (std::size_t i = 0, p = 0; i < modes.size(); ++i) {
			Mode mode = getMode(modes[i]);

			if (!mode) {
				mServer->sendNumericReply(sender, "472", util::makeVector<std::string>(modes.substr(i, 1), "is unknown mode char to me for " + mName));
				return;
			} else if (mode == CMODE_OPERATOR) {
				if (p + 1 > params.size()) {
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
				if (p + 1 > params.size()) {
					mServer->sendNumericReply(sender, "461", util::makeVector<std::string>("MODE", "Not enough parameters"));
					return;
				}

				if (params[p] == "*!*@*") {
					newBans.push_back(sender->getUsername());
				} else {
					newBans.push_back(params[p]);
				}
				++p;

			} else {
				finalMode = finalMode | mode;
			}
		}

		mMode = finalMode;

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

		std::vector<std::string> rpl_params = util::makeVector(mName, (addMode ? "+" : "-") + modes);
		rpl_params.insert(rpl_params.end(), params.begin(), params.end());

		for (std::map<UserPtr, bool>::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
			mServer->sendMessage(it->first, sender->getOrigin(), "MODE", rpl_params);
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
			if (mMode & i) {
				os << getModeChar(static_cast<ChannelMode>(i));
			}
		}

		return os.str();
	}

	char Channel::getModeChar(ChannelMode mode) {
		switch (mode) {
			case CMODE_OPERATOR:					return 'o';
			// case CMODE_PRIVATE:						return 'p';
			// case CMODE_SECRET:						return 's';
			case CMODE_INVITE:						return 'i';
			case CMODE_TOPIC_OP_ONLY:				return 't';
			case CMODE_BAN:							return 'b';
			default:								return '\0';
		}
	}

	Channel::ChannelMode Channel::getMode(char c) {
		switch (c) {
			case 'o':			return CMODE_OPERATOR;
			// case 'p':			return CMODE_PRIVATE;
			// case 's':			return CMODE_SECRET;
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

	void Channel::partMessage(UserPtr user, std::string message) {
		if (!isInChannel(user)) {
			mServer->sendNumericReply(user, "441", util::makeVector<std::string>(user->getNickname(), mName, "They aren't on that channel"));
			return;
		}

		// try {
			for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
				mServer->sendMessage(it->first, user->getOrigin(), "PART", util::makeVector(mName, message), true);
			}
		// } catch (...) {}
		mUsers.erase(user);
		user->kickedFromChannel(this);

		if (mUsers.empty()) {
			mServer->channelReclaiming(mName);
			delete this;
		}
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
				std::string() + "H" /*+ ((mMode & CMODE_PRIVATE) ? "*" : "")*/ + (uit->second ? "@" : ""),
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
					/*(mMode & CMODE_PRIVATE) ? "*" : (mMode & CMODE_SECRET) ? "@" :*/ "=",
					mName,
					(it->second ? "@" : "") + it->first->getNickname()
			));
		}

		mServer->sendNumericReply(user, "366", "End of NAMES list");
	}

	void Channel::inviteMessage(UserPtr user, std::string nickname, UserPtr target) {
		if (mUsers.count(user) == 0) {
			mServer->sendNumericReply(user, "442", util::makeVector<std::string>(mName, "You're not on that channel"));
			return;
		}

		if ((mMode & CMODE_INVITE) && !isOperator(user)) {
			mServer->sendNumericReply(user, "482", util::makeVector<std::string>(mName, "You're not channel operator"));
			return;
		}

		if (!target) {
			mServer->sendNumericReply(user, "401", util::makeVector<std::string>(nickname, "No such nick/channel"));
			return;
		}

		if (mUsers.count(target) != 0) {
			mServer->sendNumericReply(user, "443", util::makeVector<std::string>(nickname, mName, "is already on channel"));
			return;
		}

		mInviteList.insert(nickname);

		mServer->sendMessage(target, user->getOrigin(), "INVITE", util::makeVector(nickname, mName));
		mServer->sendNumericReply(user, "341", util::makeVector<std::string>(mName, nickname));
	}

	void Channel::kickMessage(UserPtr user, std::vector<std::string> targets, std::string &comment) {
		if (mUsers.count(user) == 0) {
			mServer->sendNumericReply(user, "442", util::makeVector<std::string>(mName, "You're not on that channel"));
			return;
		}

		if (!isOperator(user)) {
			mServer->sendNumericReply(user, "482", util::makeVector<std::string>(mName, "You're not channel operator"));
			return;
		}

		for (std::size_t i = 0; i < targets.size(); ++i) {
			std::string nickname = targets[i];
			UserPtr curr = mServer->getUser(nickname);

			if (!curr) {
				mServer->sendNumericReply(user, "401", util::makeVector<std::string>(nickname, "No such nick/channel"));
				continue;
			}

			if (mUsers.count(curr) == 0) {
				mServer->sendNumericReply(user, "441", util::makeVector<std::string>(nickname, mName, "They aren't on that channel"));
				continue;
			}

			for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
				mServer->sendMessage(it->first, user->getOrigin(), "KICK", util::makeVector(mName, nickname, comment), true);
			}

			mUsers.erase(curr);
			curr->kickedFromChannel(this);
		}

		if (mUsers.empty()) {
			mServer->channelReclaiming(mName);
			delete this;
		}
	}

	void Channel::answerList(UserPtr user) const {
		std::string userCount;

		// if ((mMode & CMODE_PRIVATE) && mUsers.count(user) == 0) {
			userCount = "Prv";
		// } else {
			std::stringstream ss;

			ss << mUsers.size();
			userCount = ss.str();
		// }


		// Channel name, User count/Prv (private), Topic (may be empty, but MUST be present)
		mServer->sendNumericReply(user, "322", util::makeVector(mName, userCount, mTopic));
	}

	bool Channel::sendMessage(UserPtr sender, internal::Message message) {
		if (mUsers.count(sender) == 0) {
			if (!message.isNotice()) mServer->sendNumericReply(sender, "404", util::makeVector<std::string>(mName, "Cannot send to channel"));
			return false;
		}

		message.trySetChannel(mName);

		for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
			if (it->first != sender) {
				it->first->sendMessage(message);
			}
		}

		if (message.getMessage() == "!blague") {
			std::string &joke = mServer->nextJoke();

			internal::Message jokeMessage(internal::Origin("joke_bot"), joke, mName, true);

			for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
				it->first->sendMessage(jokeMessage);
			}
		}

		return true;
	}

	bool Channel::kickUser(UserPtr kicked) {
		return !!(mUsers.erase(kicked)) && kicked->kickedFromChannel(this);
	}

	void Channel::userWillRename(UserPtr user, std::string newNick) {
		for (user_storage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
			if (it->first == user) {
				continue;
			}

			mServer->sendMessage(it->first, user->getOrigin(), "NICK", newNick);
		}

		// std::set<std::string>::iterator ban = mBanList.find(user->getNickname());

		// if (ban != mBanList.end()) {
		// 	mBanList.erase(ban);
		// 	mBanList.insert(newNick);
		// }

		// std::set<std::string>::iterator invite = mInviteList.find(user->getNickname());

		// if (invite != mInviteList.end()) {
		// 	mInviteList.erase(invite);
		// 	mInviteList.insert(newNick);
		// }
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
