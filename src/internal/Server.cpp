#include <internal/Server.hpp>
#include <data/User.hpp>
#include <data/Channel.hpp>

#include <util/Util.hpp>

#include <iostream>
#include <stdexcept>

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
		api::IComm *commAPI = getCommInterface();

		if (!user) {
			return false;
		}

		if (command == "PASS") {
			if (!requiresParam(fd, command, params, 1))
				return true;

			if (user->getAuthenticated()) {
				return sendError(fd, "462", util::makeVector<std::string>("You may not reregister"));
			}

			user->setSentPassword(params[0]);
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
} // namespace internal
