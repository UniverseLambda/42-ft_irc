#include <internal/Message.hpp>

namespace internal {
	Message::Message(Origin origin, std::string message):
		mOrigin(origin),
		mMessage(message) {}

	Message::Message(Origin origin, std::string message, std::string channel):
		mOrigin(origin),
		mMessage(message),
		mChannel(channel) {}

	Message::Message(const Message &orig):
		mOrigin(orig.mOrigin),
		mMessage(orig.mMessage),
		mChannel(orig.mChannel) {}

	Message::~Message() {}

	Message &Message::operator=(const Message &orig) {
		mOrigin = orig.mOrigin;
		mMessage = orig.mMessage;
		mChannel = orig.mChannel;

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
		return (os << message.getOrigin().toString() << " " << message.getChannel() << ":" << message.getMessage());
	}
} // namespace internal
