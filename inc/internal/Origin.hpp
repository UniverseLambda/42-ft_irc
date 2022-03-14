#pragma once

#include <string>

#include <util/Optional.hpp>

namespace internal {
	class Origin {
	private:
		std::string mNickServer;
		util::Optional<std::string> mUser;
		util::Optional<std::string> mHost;

	public:
		Origin();
		Origin(std::string nickServer);
		Origin(std::string nick, std::string user);
		Origin(std::string nick, std::string user, std::string host);
		Origin(const Origin &orig);
		~Origin();

		Origin &operator=(const Origin &orig);

		std::string &servername();
		const std::string &servername() const;

		std::string &nickname();
		const std::string &nickname() const;

		util::Optional<std::string> &user();
		const util::Optional<std::string> &user() const;

		util::Optional<std::string> &host();
		const util::Optional<std::string> &host() const;

		std::string toString() const;

		bool operator==(const Origin &other) const;
	};
} // namespace internal
