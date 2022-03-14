#include <internal/Origin.hpp>
#include <sstream>

namespace internal {
	Origin::Origin() {}
	Origin::Origin(std::string nickServer): mNickServer(nickServer) {}
	Origin::Origin(std::string nick, std::string user): mNickServer(nick), mUser(user) {}
	Origin::Origin(std::string nick, std::string user, std::string host): mNickServer(nick), mUser(user), mHost(host) {}
	Origin::Origin(const Origin &orig): mNickServer(orig.mNickServer), mUser(orig.mUser), mHost(orig.mHost) {}
	Origin::~Origin() {}

	Origin &Origin::operator=(const Origin &orig) {
		mNickServer = orig.mNickServer;
		mUser = orig.mUser;
		mHost = orig.mHost;

		return (*this);
	}

	std::string &Origin::servername() {
		return mNickServer;
	}

	const std::string &Origin::servername() const {
		return mNickServer;
	}

	std::string &Origin::nickname() {
		return mNickServer;
	}

	const std::string &Origin::nickname() const {
		return mNickServer;
	}

	util::Optional<std::string> &Origin::user() {
		return mUser;
	}

	const util::Optional<std::string> &Origin::user() const {
		return mUser;
	}

	util::Optional<std::string> &Origin::host() {
		return mHost;
	}

	const util::Optional<std::string> &Origin::host() const {
		return mHost;
	}

	std::string Origin::toString() const {
		std::ostringstream oss;

		oss << mNickServer;

		if (mUser) {
			oss << "!" << mUser.unwrap();
		}

		if (mHost) {
			oss << "@" << mHost.unwrap();
		}

		return oss.str();
	}

	bool Origin::operator==(const Origin &other) const {
		return
			mNickServer == other.mNickServer &&
			mUser == other.mUser &&
			mHost == other.mHost;
	}
} // namespace internal
