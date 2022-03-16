#include <internal/Server.hpp>
#include <data/User.hpp>
#include <data/Channel.hpp>

#include <util/Util.hpp>

#include <iostream>
#include <stdexcept>
#include <sstream>

#include <locale>

namespace internal {
	Server::Server() {}

	Server::Server(std::string password, api::IComm *comm): mPassword(password), mCommInterface(comm) {}

	Server::Server(const Server &orig):
		mPassword(orig.mPassword),
		mCommInterface(orig.mCommInterface),
		mUsers(orig.mUsers),
		mChannels(orig.mChannels) {}

	Server::~Server() {
		for (std::map<int, data::UserPtr>::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
			delete it->second;
		}

		for (std::map<std::string, data::ChannelPtr>::iterator it = mChannels.begin(); it != mChannels.end(); ++it) {
			delete it->second;
		}
	}

	Server &Server::operator=(const Server &orig) {
		mPassword = orig.mPassword;
		mCommInterface = orig.mCommInterface;
		mUsers = orig.mUsers;
		mChannels = orig.mChannels;
		return *this;
	}

	data::UserPtr Server::addUser(int fd) {
		data::UserPtr user = new data::User(fd, this);

		if (!mUsers.insert(std::make_pair(fd, user)).second) {
			std::cerr << "Woops! Duplicate fd: " << fd << std::endl;
			throw std::runtime_error("Woops. Duplicate fd");
		}

		return user;
	}

	bool Server::removeUser(int fd) {
		return mUsers.erase(fd) != 0;
	}

	std::string Server::getPassword() const {
		return mPassword;
	}

	data::UserPtr Server::getUser(int fd) const {
		try {
			return mUsers.at(fd);
		} catch (...) {}

		return NULL;
	}


	data::ChannelPtr Server::getChannel(std::string name) const {
		try {
			return mChannels.at(name);
		} catch (...) {}

		return NULL;
	}

	data::ChannelPtr Server::getOrCreateChannel(std::string name) {
		try {
			data::ChannelPtr chan = getChannel(name);

			if (chan == NULL) {
				chan = new data::Channel(name, this);
				mChannels.insert(std::make_pair(name, chan));
			}

			return chan;
		} catch (...) {}
		return NULL;
	}

	api::IComm *Server::getCommInterface() const {
		return mCommInterface;
	}

	void Server::channelReclaiming(std::string name) {
		mChannels.erase(name);
	}

	bool Server::userDisconnected(int fd) {
		std::map<int, data::UserPtr>::iterator user = mUsers.find(fd);

		if (user == mUsers.end()) {
			return false;
		}

		data::UserPtr ptr = user->second;

		ptr->dispatchDisconnect();
		mUsers.erase(user);
		delete ptr;

		return true;
	}

	bool Server::admitMessage(int fd, std::string command, std::vector<std::string> params) {
		data::UserPtr user = getUser(fd);
		// api::IComm *commAPI = getCommInterface();

		if (!user) {
			return false;
		}

		if (command == "PASS") {
			if (!requiresParam(fd, command, params, 1))
				return true;

			if (user->isAuthenticated()) {
				return sendError(fd, "462", util::makeVector<std::string>("You may not reregister"));
			}

			user->setSentPassword(params[0]);

			return true;
		} else if (command == "NICK") {
			if (params.empty()) {
				return sendError(fd, "431", util::makeVector<std::string>("No nickname given"));
			}

			std::string &nick = params[0];

			if (!checkNickname(nick)) {
				return sendError(fd, "432", util::makeVector<std::string>("Erroneus nickname"));
			}

			for (userStorage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
				if (it->second->getNickname() == nick) {
					return sendError(fd, "433", util::makeVector<std::string>(nick, "Nickname is already in use"));
				}
			}

			user->setNickname(nick);
			return tryToAuthenticate(user);
		} else if (command == "USER") {
			if (!requiresParam(fd, command, params, 4))
				return true;

			if (user->isAuthenticated()) {
				return sendError(fd, "462", util::makeVector<std::string>("You may not reregister"));
			}

			user->setUsername(params[0]);
			user->setHostname(params[1]);
			user->setRealname(params[3]);

			return tryToAuthenticate(user);
		}

		if (!user->isAuthenticated()) {
			return sendError(fd, "451", util::makeVector<std::string>("You have not registered"));
		}

		return false;
	}

	bool Server::requiresParam(int fd, std::string command, std::vector<std::string> params, std::size_t count) {
		if (params.size() < count) {
			sendError(fd, "461", util::makeVector<std::string>(command, "Not enough parameters"));
			return false;
		}
		return true;
	}

	bool Server::sendError(int fd, std::string errorCode, std::vector<std::string> params) const {
		return getCommInterface()->sendMessage(fd, util::Optional<Origin>(), errorCode, params, true);
	}

	bool Server::tryToAuthenticate(data::UserPtr user) {
		if (user->isAuthenticated()) {
			return true;
		}

		if (user->getNickname().empty() || user->getUsername().empty()) {
			return true;
		}

		if (user->getSentPassword() != getPassword()) {
			return sendError(user->getFd(), "464", util::makeVector<std::string>("Password incorrect"));
		}

		user->setAuthenticated(true);

		// MOTD (or NOTD mdrrrrrrrrrr kill me please)

		// RPL_LUSER*
		if (!handleLUsers(user->getFd())) {
			return false;
		}

		// RPL_VERSION
		if (!sendError(user->getFd(), "351", util::makeVector<std::string>("1.0", "IRFun", "no comment, enjoy our crash"))) {
			return false;
		}

		return true;
	}

	bool Server::handleLUsers(int fd) const {
		std::ostringstream os;

		std::string luserClient;
		std::string luserChannels;
		std::string luserMe;

		std::size_t invisible = 0;

		for (userStorage::const_iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
			invisible += !!(it->second->getMode() & data::User::UMODE_INVISIBLE);
		}

		os << "There are " << mUsers.size() << " users and " << invisible << " invisible on 1 servers";
		luserClient = os.str();
		os.clear();

		if (!mChannels.empty()) {
			os << mChannels.size();
			luserChannels = os.str();
			os.clear();
		}


		os << "I have " << mUsers.size() << " clients and 1 servers";
		luserClient = os.str();
		os.clear();

		return
			sendError(fd, "251", util::makeVector<std::string>(luserClient))
			&& (luserChannels.empty() || sendError(fd, "254", util::makeVector<std::string>(luserChannels)))
			&& sendError(fd, "255", util::makeVector<std::string>(luserClient))
		;
	}

	bool Server::checkNickname(const std::string &nick) {
		if (!nick.empty())
			return false;

		if (!std::isalpha(nick[0]))
			return false;

		for (std::size_t i = 1; i < nick.length(); ++i) {
			char c = nick[i];
			if (!std::isalnum(c)
				&& c != '-' && c != '[' && c != ']' && c != '\\' && c != '`' &&  c != '^' && c != '{' && c != '}')
				return false;
		}

		return true;
	}
} // namespace internal
