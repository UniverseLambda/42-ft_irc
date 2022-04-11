#pragma once

#include <internal/Forward.hpp>
#include <data/Forward.hpp>

#include <api/IComm.hpp>

#include <util/Util.hpp>

#include <map>
#include <string>
#include <vector>

namespace internal {
	class Server {
	private:
		typedef std::map<int, data::UserPtr> userStorage;
		typedef std::map<std::string, data::ChannelPtr> channelStorage;

		std::string mPassword;
		api::IComm *mCommInterface;
		userStorage mUsers;
		channelStorage mChannels;

		std::size_t mJokeCounter;
		std::vector<std::string> mJokes;

	public:
		Server();
		Server(std::string password, api::IComm *comm);
		Server(const Server &orig);
		~Server();

		Server &operator=(const Server &orig);

		data::UserPtr addUser(int fd);
		bool removeUser(int fd);

		static std::string getHost();

		std::string getPassword() const;
		data::UserPtr getUser(int fd) const;
		data::UserPtr getUser(std::string nickname) const;
		data::ChannelPtr getChannel(std::string name) const;

		data::ChannelPtr getOrCreateChannel(std::string name);

		api::IComm *getCommInterface() const;

		void channelReclaiming(std::string name);

		bool userDisconnected(int fd);
		void userDisconnected(data::UserPtr user, std::string message = "Disconnected");

		bool admitMessage(int fd, std::string command, std::vector<std::string> params = std::vector<std::string>());

		bool sendNumericReply(data::UserPtr user, std::string code, std::string param) const;
		bool sendNumericReply(data::UserPtr user, std::string code, std::vector<std::string> params) const;
		bool sendMessage(data::UserPtr user, util::Optional<internal::Origin> prefix, std::string command, std::string params, bool lastParamExtended = false) const;
		bool sendMessage(data::UserPtr user, util::Optional<internal::Origin> prefix, std::string command, std::vector<std::string> params = std::vector<std::string>(), bool lastParamExtended = false) const;

		std::string &nextJoke();

	private:
		bool requiresParam(data::UserPtr user, std::string command, std::vector<std::string> params, std::size_t count);

		bool tryToAuthenticate(data::UserPtr user);

		bool handleLUsers(data::UserPtr user) const;

		bool handleMode(data::UserPtr user, std::vector<std::string> params);

		static bool checkNickname(const std::string &nick);
	};
} // namespace internal
