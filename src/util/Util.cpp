#include <util/Util.hpp>
#include <internal/Server.hpp>

namespace util {
	std::vector<std::string> parseList(std::string list) {
		std::vector<std::string> members;

		for (std::size_t pos = 0; list.find(',') != std::string::npos;) {
			members.push_back(list.substr(0, pos));
			list.erase(0, pos + 1);
		}

		return members;
	}

	bool sendNumericReply(api::IComm *comm, data::UserPtr user, std::string code, std::string param) {
		return comm->sendMessage(user->getFd(), internal::Origin(internal::Server::getHost()), code, makeVector(param), true);
	}

	bool sendNumericReply(api::IComm *comm, data::UserPtr user, std::string code, std::vector<std::string> params) {
		params.insert(params.begin(), user->getNickname());
		return comm->sendMessage(user->getFd(), internal::Origin(internal::Server::getHost()), code, params, true);
	}
} // namespace util
