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

		std::string mPassword;
		api::IComm *mCommInterface;
		userStorage mUsers;
		std::map<std::string, data::ChannelPtr> mChannels;

	public:
		Server();
		Server(std::string password, api::IComm *comm);
		Server(const Server &orig);
		~Server();

		Server &operator=(const Server &orig);

		data::UserPtr addUser(int fd);
		bool removeUser(int fd);

		std::string getPassword() const;
		data::UserPtr getUser(int fd) const;
		data::ChannelPtr getChannel(std::string name) const;

		data::ChannelPtr getOrCreateChannel(std::string name);

		api::IComm *getCommInterface() const;

		void channelReclaiming(std::string name);

		bool userDisconnected(int fd);
		void userDisconnected(data::UserPtr user, std::string message = "Disconnected");

		bool admitMessage(int fd, std::string command, std::vector<std::string> params = std::vector<std::string>());

	private:

		bool requiresParam(int fd, std::string command, std::vector<std::string> params, std::size_t count);
		bool sendError(int fd, std::string errorCode, std::vector<std::string> params) const;

		bool tryToAuthenticate(data::UserPtr user);

		bool handleLUsers(int fd) const;

		bool handleMode(data::UserPtr user, std::vector<std::string> params);

		template<typename _Tp>
		void genericHandleMode(data::UserPtr user, _Tp *target, std::vector<std::string> params) {
			// typedef typename _Tp::Mode Mode;

			if (params.size() == 1) {
				util::sendNumericReply(getCommInterface(), user, "221", util::makeVector("+" + target->getModeString()));
				return;
			}
		}

		static bool checkNickname(const std::string &nick);
	};
} // namespace internal
