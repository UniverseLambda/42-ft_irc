#pragma once

#include "Interface.hpp"

#include <string>
#include <vector>

namespace api {
	struct IComm {
		FT_INTERFACE_PRELUDE(IComm);

		virtual bool sendMessage(int fd, std::string command, std::vector<std::string> parameters = std::vector<std::string>(), bool lastParamExtended = false) = 0;
	};
} // namespace api
