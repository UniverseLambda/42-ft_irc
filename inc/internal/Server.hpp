#pragma once

#include <internal/Forward.hpp>
#include <data/Forward.hpp>

#include <api/IComm.hpp>

#include <map>
#include <string>
#include <vector>

namespace internal {
	class Server {
	private:
		std::string mPassword;
		api::IComm *mCommInterface;
		std::map<int, data::UserPtr> mUsers;
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

		bool admitMessage(int fd, std::string command, std::vector<std::string> params = std::vector<std::string>());

		api::IComm *getCommInterface() const;

		void channelReclaiming(std::string name);

		bool userDisconnected(int fd);
	};
} // namespace internal
