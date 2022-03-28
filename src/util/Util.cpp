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
} // namespace util
