#include <internal/Message.hpp>

namespace internal {
	Message::Message(Origin origin, std::string message, bool notice):
		mOrigin(origin),
		mMessage(message),
		mNotice(notice) {}

	Message::Message(Origin origin, std::string message, std::string channel, bool notice):
		mOrigin(origin),
		mMessage(message),
		mChannel(channel),
		mNotice(notice) {}

	Message::Message(const Message &orig):
		mOrigin(orig.mOrigin),
		mMessage(orig.mMessage),
		mChannel(orig.mChannel),
		mNotice(orig.mNotice) {}

	Message::~Message() {}

	Message &Message::operator=(const Message &orig) {
		mOrigin = orig.mOrigin;
		mMessage = orig.mMessage;
		mChannel = orig.mChannel;
		mNotice = orig.mNotice;

		return (*this);
	}

	Origin Message::getOrigin() const {
		return mOrigin;
	}

	std::string Message::getMessage() const {
		return mMessage;
	}

	std::string Message::getChannel() const {
		return mChannel;
	}

	bool Message::isNotice() const {
		return mNotice;
	}

	bool Message::hasChannel() const {
		return !mChannel.empty();
	}

	bool Message::trySetChannel(std::string channel) {
		if (hasChannel()) {
			return false;
		}

		mChannel = channel;

		return true;
	}

	std::ostream &operator<<(std::ostream &os, const Message &message) {
		return (os << message.getOrigin().toString() << " " << message.getChannel() << " (" << (message.isNotice() ? "NOTICE" : "PRIVMSG") << ")" << ":" << message.getMessage());
	}
} // namespace internal
