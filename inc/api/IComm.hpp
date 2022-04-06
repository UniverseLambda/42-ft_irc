#pragma once

#include "Interface.hpp"

#include <string>
#include <vector>
#include <util/Optional.hpp>
#include <internal/Origin.hpp>

namespace api {
	struct IComm {
		FT_INTERFACE_PRELUDE(IComm);

		virtual bool stockMessage(int fd, util::Optional<internal::Origin> prefix, std::string command, std::vector<std::string> parameters = std::vector<std::string>(), bool lastParamExtended = false) = 0;
	};
} // namespace api
