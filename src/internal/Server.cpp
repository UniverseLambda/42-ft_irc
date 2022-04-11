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

	Server::Server(std::string password, api::IComm *comm): mPassword(password), mCommInterface(comm), mJokeCounter(0), mJokes() {
		mJokes.push_back("Quelle princesse a les levres gercees ? Labello bois dormant");
		mJokes.push_back("Que dit un rappeur quand il rentre dans une fromagerie ? Faites du brie");
		mJokes.push_back("Quelle maladie peuvent attraper les chats ? La minou-nucleose");
		mJokes.push_back("Quel poisson n'a pas de date d'anniversaire ? Le poisson pane");
		mJokes.push_back("Quel legume pousse sous l'eau ? Le chou-marin");
	}

	Server::Server(const Server &orig):
		mPassword(orig.mPassword),
		mCommInterface(orig.mCommInterface),
		mUsers(orig.mUsers),
		mChannels(orig.mChannels),
		mJokeCounter(orig.mJokeCounter),
		mJokes(orig.mJokes) {}

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
		mJokeCounter = orig.mJokeCounter;
		mJokes = orig.mJokes;
		return *this;
	}

	data::UserPtr Server::addUser(int fd) {
		std::pair<internal::Server::userStorage::iterator, bool> result = mUsers.insert(std::make_pair<int, data::UserPtr>(fd, NULL));

		if (!result.second) {
			std::cerr << "Woops! Duplicate fd: " << fd << std::endl;
			throw std::runtime_error("Woops. Duplicate fd");
		}

		data::UserPtr user = new data::User(fd, this);
		result.first->second = user;

		return user;
	}

	bool Server::removeUser(int fd) {
		return mUsers.erase(fd) != 0;
	}

	std::string Server::getHost() {
		return "irfun.fr";
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

	data::UserPtr Server::getUser(std::string nickname) const {
		for (userStorage::const_iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
			if (it->second->getNickname() == nickname) {
				return it->second;
			}
		}

		return NULL;
	}

	data::ChannelPtr Server::getChannel(std::string name) const {
		try {
			return mChannels.at(name);
		} catch (...) {}

		return NULL;
	}

	data::ChannelPtr Server::getOrCreateChannel(std::string name) {
		if (name.size() < 2 || (name[0] != '#' && name[0] != '&')) {
			return NULL;
		}

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

		userDisconnected(user->second);
		return true;
	}

	void Server::userDisconnected(data::UserPtr user, std::string message) {
		user->dispatchDisconnect(message);

		mUsers.erase(user->getFd());
		delete user;
	}

	bool Server::admitMessage(int fd, std::string command, std::vector<std::string> params) {
		data::UserPtr user;

		std::cout << "[fd:" << fd << "]: " << command << std::endl;

		if (!(user = getUser(fd))) {
			user = addUser(fd);
		}

		if (command == "CAP") {
			return sendNumericReply(user, "421", util::makeVector<std::string>(command, "Unknown command"));
		}

		if (command == "PASS") {
			if (!requiresParam(user, command, params, 1))
				return true;

			if (user->isAuthenticated()) {
				return sendNumericReply(user, "462", "You may not reregister");
			}

			user->setSentPassword(params[0]);

			return true;
		} else if (command == "NICK") {
			if (params.empty()) {
				return sendNumericReply(user, "431", "No nickname given");
			}

			std::string &nick = params[0];

			if (!checkNickname(nick)) {
				return sendNumericReply(user, "432", "Erroneus nickname");
			}

			for (userStorage::iterator it = mUsers.begin(); it != mUsers.end(); ++it) {
				if (it->second->getNickname() == nick) {
					return sendNumericReply(user, "433", util::makeVector<std::string>(nick, "Nickname is already in use"));
				}
			}


			if (user->isAuthenticated()) {
				user->dispatchWillRename(nick);
				return true;
			} else {
				user->setNickname(nick);
				return tryToAuthenticate(user);
			}
		} else if (command == "USER") {
			if (!requiresParam(user, command, params, 4))
				return true;

			if (user->isAuthenticated()) {
				return sendNumericReply(user, "462", "You may not reregister");
			}

			user->setUsername(params[0]);
			user->setHostname(params[2]);
			user->setRealname(params[3]);

			return tryToAuthenticate(user);
		}

		if (!user->isAuthenticated()) {
			return sendNumericReply(user, "451", "You have not registered");
		}

		if (command == "QUIT") {
			if (params.empty()) {
				userDisconnected(user);
			} else {
				userDisconnected(user, params[0]);
			}
		} else if (command == "JOIN") {
			if (!requiresParam(user, "JOIN", params, 1))
				return true;

			std::vector<std::string> channels = util::parseList(params[0]);

			for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it) {
				data::ChannelPtr channel = getOrCreateChannel(*it);

				if (!channel) {
					return sendNumericReply(user, "403", util::makeVector<std::string>(*it, "No such channel"));
				}

				channel->userJoin(user);
			}
		} else if (command == "PART") {
			if (!requiresParam(user, command, params, 1)) {
				return true;
			}

			std::vector<std::string> channels = util::parseList(params[0]);
			std::string message = (params.size() > 1) ? params[1] : "Parting";

			for (std::size_t i = 0; i < channels.size(); ++i) {
				data::ChannelPtr channel = getChannel(channels[i]);

				if (!channel) {
					sendNumericReply(user, "403", util::makeVector<std::string>(channels[i], "No such channel"));
					continue;
				}

				channel->partMessage(user, message);
			}
		} else if (command == "MODE") {
			if (!requiresParam(user, command, params, 1))
				return true;

			return handleMode(user, params);
		} else if (command == "TOPIC") {
			if (!requiresParam(user, command, params, 1)) {
				return true;
			}

			std::string &channelName = params[0];

			try {
				if (params.size() >= 2) {
					mChannels.at(channelName)->topicMessage(user, params[1]);
				} else {
					mChannels.at(channelName)->topicMessage(user);
				}

			} catch (...) {
				return sendNumericReply(user, "403", util::makeVector<std::string>(channelName, "No such channel"));
			}
		} else if (command == "WHO") {
			if (!requiresParam(user, command, params, 1)) {
				return true;
			}

			std::string &channelName = params[0];

			try {
				mChannels.at(channelName)->whoMessage(user, params[0]);
			} catch (...) {
				return sendNumericReply(user, "403", util::makeVector<std::string>(channelName, "No such channel"));
			}
		} else if (command == "INVITE") {
			if (!requiresParam(user, command, params, 2)) {
				return true;
			}

			std::string &nickname = params[0];
			std::string &channelName = params[1];
			data::UserPtr target = getUser(nickname);

			try {
				mChannels.at(channelName)->inviteMessage(user, nickname, target);
			} catch (...) {
				return sendNumericReply(user, "403", util::makeVector<std::string>(channelName, "No such channel"));
			}
		} else if (command == "KICK") {
			if (!requiresParam(user, command, params, 2)) {
				return true;
			}

			std::vector<std::string> channelNames = util::parseList(params[0]);
			std::vector<std::string> nicknames = util::parseList(params[1]);
			std::string comment = (params.size() >= 3) ? params[2] : "kicked";

			if (channelNames.size() != 1 && channelNames.size() != nicknames.size()) {
				return sendNumericReply(user, "461", util::makeVector<std::string>(command, "Not enough parameters"));
			}

			for (std::size_t i = 0; i < channelNames.size(); ++i) {
				std::string channel = channelNames[i];

				try {
					mChannels.at(channel)->kickMessage(
						user,
						(channelNames.size() > 1)
							? std::vector<std::string>(nicknames.begin() + i, nicknames.begin() + i + 1)
							: nicknames,
						comment
					);
				} catch (...) {
					return sendNumericReply(user, "403", util::makeVector<std::string>(channel, "No such channel"));
				}
			}
		} else if (command == "PRIVMSG" || command == "NOTICE") {
			bool notice = (command == "NOTICE");

			if (params.size() == 0 || params[0].empty()) {
				return notice || sendNumericReply(user, "411", "No recipient given (" + command + ")");
			} else if (params.size() == 1 || params[1].empty()) {
				return notice || sendNumericReply(user, "412", "No text to send");
			}

			std::string &target = params[0];
			std::string &message = params[1];

			if (target[0] == '#' || target[1] == '&') {
				// Channel
				data::ChannelPtr chan = getChannel(target);
				if (!chan) {
					return notice || sendNumericReply(user, "401", util::makeVector<std::string>(target, "No such nick/channel"));
				}

				return chan->sendMessage(user, internal::Message(user->getOrigin(), message, notice));
			} else {
				// User
				data::UserPtr tuser = getUser(target);

				if (!tuser) {
					return notice || sendNumericReply(user, "401", util::makeVector<std::string>(target, "No such nick/channel"));
				}

				return tuser->sendMessage(internal::Message(user->getOrigin(), message, notice));
			}
		} else if (command == "PING") {
			if (params.size() == 0) {
				return sendNumericReply(user, "409", "No origin specified");
			} else if (params.size() > 1 && params[1] != getHost()) {
				return sendNumericReply(user, "402", util::makeVector<std::string>(params[1], "No such server"));
			}

			return sendMessage(user, Origin(getHost()), "PONG", util::makeVector(getHost(), params[0]), true);
		} else if (command == "LIST") {
			if (params.size() >= 2 && params[1] != getHost()) {
				return sendNumericReply(user, "402", util::makeVector<std::string>(params[1], "No such server"));
			}

			if (params.size() >= 1) {
				std::vector<std::string> channelNames = util::parseList(params[0]);

				for (std::size_t i = 0; i < channelNames.size(); ++i) {
					data::ChannelPtr chan = getChannel(channelNames[i]);

					if (!chan)
						continue;
					chan->answerList(user);
				}
			} else {
				for (channelStorage::iterator it = mChannels.begin(); it != mChannels.end(); ++it) {
					it->second->answerList(user);
				}
			}

			return sendNumericReply(user, "323", "End of LIST");
		} else {
			return sendNumericReply(user, "421", util::makeVector<std::string>(command, "Unknown command"));
		}

		return true;
	}

	bool Server::requiresParam(data::UserPtr user, std::string command, std::vector<std::string> params, std::size_t count) {
		if (params.size() < count) {
			sendNumericReply(user, "461", util::makeVector<std::string>(command, "Not enough parameters"));
			return false;
		}
		return true;
	}

	bool Server::sendNumericReply(data::UserPtr user, std::string code, std::string param) const {
		return sendNumericReply(user, code, util::makeVector(param));
	}

	bool Server::sendNumericReply(data::UserPtr user, std::string code, std::vector<std::string> params) const {
		params.insert(params.begin(), user->getNickname());
		return sendMessage(user, Origin(getHost()), code, params, true);
	}

	bool Server::sendMessage(data::UserPtr user, util::Optional<internal::Origin> prefix, std::string command, std::string param, bool lastParamExtended) const {
		return sendMessage(user, prefix, command, util::makeVector(param), lastParamExtended);
	}

	bool Server::sendMessage(data::UserPtr user, util::Optional<internal::Origin> prefix, std::string command, std::vector<std::string> params, bool lastParamExtended) const {
		if (mCommInterface) {
			return mCommInterface->stockMessage(user->getFd(), prefix, command, params, lastParamExtended);
		}
		std::cerr << "NO ICOMM SET" << std::endl;
		std::cerr << "MESSAGE CONTENT: " << std::endl;
		std::cerr << " user:   " << user->getFd() << std::endl;
		std::cerr << " prefix: " << prefix << std::endl;
		std::cerr << " command: " << command;

		for (std::size_t i = 0; i < params.size(); ++i) {
			std::cerr << " ";
			if (i + 1 == params.size() && lastParamExtended)
				std::cerr << ":";
			std::cerr << params[i];
		}

		std::cerr << std::endl;
		return false;
	}

	std::string &Server::nextJoke() {
		std::string &joke = mJokes[mJokeCounter];

		mJokeCounter = (mJokeCounter + 1) % mJokes.size();

		return joke;
	}

	bool Server::tryToAuthenticate(data::UserPtr user) {
		if (user->isAuthenticated()) {
			return true;
		}

		if (user->getNickname().empty() || user->getUsername().empty()) {
			return true;
		}

		if (user->getSentPassword() != getPassword()) {
			return sendNumericReply(user, "464", "Password incorrect");
		}

		user->setAuthenticated(true);

		// 001 RPL_WELCOME
		sendNumericReply(user, "001", "Welcome to the Internet Relay Network " + user->getOrigin().toString());

		// 002 RPL_YOURHOST
		sendNumericReply(user, "002", "Your host is " + getHost() + ", running version irfun-1.0");

		// 003 RPL_CREATED
		sendNumericReply(user, "003", "This server was created Thu Mar 24 2022 12:37 (CET)");

		// 004 RPL_MYINFO
		sendNumericReply(user, "004", util::makeVector<std::string>(getHost(), "irfun-1.0", "*", "otib"));

		// RPL_LUSER
		handleLUsers(user);

		// MOTD (or NOTD mdrrrrrrrrrr kill me please)
		sendNumericReply(user, "422", "MOTD File is missing");

		return true;
	}

	bool Server::handleLUsers(data::UserPtr user) const {
		std::string luserClient;
		std::string luserChannels;
		std::string luserMe;

		{
			std::ostringstream os;

			os << "There are " << mUsers.size() << " users and 0 invisible on 1 servers";
			luserClient = os.str();
		}

		if (!mChannels.empty()) {
			std::ostringstream os;

			os << mChannels.size();
			luserChannels = os.str();
		}

		{
			std::ostringstream os;

			os << "I have " << mUsers.size() << " clients and 1 servers";
			luserMe = os.str();
			os.clear();
		}


		return
			sendNumericReply(user, "251", luserClient)
			&& (luserChannels.empty() || sendNumericReply(user, "254", luserChannels))
			&& sendNumericReply(user, "255", luserMe)
		;
	}

	bool Server::handleMode(data::UserPtr user, std::vector<std::string> params) {
		(void)user;

		if (params[0].empty() || (params[0][0] != '&' && params[0][0] != '#')) {
			// User
			return sendNumericReply(user, "501", "Unknown MODE flag");
		}

		data::ChannelPtr target = getChannel(params[0]);

		// Checking if channel exists
		if (!target) {
			return sendNumericReply(user, "401", "No such nick/channel");
		}

		// Checking if user's in channel
		if (!target->isInChannel(user)) {
			return sendNumericReply(user, "441", util::makeVector<std::string>(user->getNickname(), target->getName(), "They aren't on that channel"));
		}

		// Simple mode request
		if (params.size() == 1) {
			return sendNumericReply(user, "324", util::makeVector<std::string>(target->getName(), "+" + target->getModeString()));
		}

		// Trying to set mode

		// Checking if user is operator
		if (!target->isOperator(user)) {
			return sendNumericReply(user, "482", util::makeVector<std::string>(target->getName(), "You're not channel operator"));
		}

		// If empty mode, then do nothing
		if (params[1].empty()) {
			return true;
		}

		bool addition = params[1][0] != '-';
		std::string modeParam = params[1].substr(params[1][0] == '+' || params[1][0] == '-');

		// If empty mode, then do nothing
		if (modeParam.empty()) {
			return true;
		}

		target->admitMode(user, modeParam, addition, std::vector<std::string>(params.begin() + 2, params.end()));
		return true;
	}

	bool Server::checkNickname(const std::string &nick) {
		if (nick.empty())
			return false;

		if (!std::isalpha(nick[0]))
			return false;

		for (std::size_t i = 1; i < nick.length(); ++i) {
			char c = nick[i];
			if (!std::isalnum(c)
				&& c != '-' && c != '[' && c != ']' && c != '\\' && c != '`' &&  c != '^' && c != '{' && c != '}' && c != '_')
				return false;
		}

		return true;
	}
} // namespace internal
