#include <util/Util.hpp>

namespace util {
	std::vector<std::string> parseList(std::string list) {
		std::vector<std::string> members;

		for (std::size_t pos = 0; list.find(',') != std::string::npos;) {
			members.push_back(list.substr(0, pos));
			list.erase(0, pos + 1);
		}

		return members;
	}

	bool sendNumericReply(api::IComm *comm, data::UserPtr user, std::string code, std::vector<std::string> params) {
		return comm->sendMessage(user->getFd(), internal::Origin(), code, params, true);
	}
} // namespace util
