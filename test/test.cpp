#include <internal/Server.hpp>
#include <internal/Message.hpp>
#include <data/User.hpp>
#include <data/Channel.hpp>

#include <api/IComm.hpp>

#include <iostream>
#include <utility>
#include <map>
#include <vector>

#include <csignal>
#include <execinfo.h>
#include <unistd.h>

#ifdef __linux__
#include <cstdlib>
#endif

#define EXEC_TEST(X) { std::cout << "[*] EXECUTING " #X "..." << std::endl; if (!X()) return 1; }

#define test_assert_equal(V, X) if (!((V) == (X))) fail_test(__FILE__, __LINE__, __FUNCTION__, "failed assertion")
#define test_assert_not_equal(V, X) if ((V) == (X)) fail_test(__FILE__, __LINE__, __FUNCTION__, "failed assertion")
#define test_assert_true(V) if (!(V)) fail_test(__FILE__, __LINE__, __FUNCTION__, "failed assertion")
#define test_expect_exception(V, X) try { V; fail_test(__FILE__, __LINE__, __FUNCTION__, "exception expected"); } \
									catch (X &e) {(void)e;} \
									catch (...) { fail_test(__FILE__, __LINE__, __FUNCTION__, "wrong exception caught"); }

#define test_prelude std::string _test_file, _test_function; int _test_line;
#define test_unexpected_exception fail_test(_test_file, _test_line, _test_function, "Caught an exception");
#define new_op { _test_file = __FILE__; _test_line = __LINE__; _test_function = __FUNCTION__; }

struct MessageReceiver: api::IComm {
	typedef internal::Message Message;

	std::map< int, std::vector<Message> > msgs;

	virtual bool stockMessage(int fd, util::Optional<internal::Origin> prefix, std::string command, std::vector<std::string> params, bool) {
		if (command != "PRIVMSG" && command != "NOTICE") {
			return true;
		}

		msgs[fd].push_back(Message(prefix.unwrap(), params.at(1), params.at(0)));
		return true;
	}

	bool sendMessage(int fd, Message message) {
		msgs[fd].push_back(message);
		return true;
	}

	Message getLastMessage(int fd) {
		return msgs[fd].back();
	}

	void removeLastMessage(int fd) {
		msgs[fd].pop_back();
	}

	bool hasMessage(int fd) {
		return !(msgs[fd].empty());
	}
};

void fail_test(std::string file, int line, std::string function, std::string reason) {
	std::cerr << file << ":" << line << ": " << function << " failed (" << reason << ")" << std::endl;
	std::abort();
}

void handle_sigs(int sig) {
	void *array[20];

	std::size_t size = backtrace(array, 20);
	std::cerr << "/!\\ Caught signal: " << sig << std::endl;
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	std::exit(sig + 128);
}

bool test_user() {
	test_prelude

	try {
		MessageReceiver receiver;
		internal::Server server("", &receiver);
		data::User user(-1, &server);

		// Testing default values
		new_op test_assert_equal(user.getNickname().empty(), true);
		new_op test_assert_equal(user.getUsername().empty(), true);
		new_op test_assert_equal(user.getRealname().empty(), true);
		new_op test_assert_equal(user.isAuthenticated(), false);

		// Testing setters
		new_op user.setNickname("MDR0");
		new_op test_assert_equal(user.getNickname().empty(), false);
		new_op test_assert_equal(user.getNickname(), "MDR0");

		new_op user.setUsername("MDR1");
		new_op test_assert_equal(user.getUsername().empty(), false);
		new_op test_assert_equal(user.getUsername(), "MDR1");

		new_op user.setRealname("MDR2");
		new_op test_assert_equal(user.getRealname().empty(), false);
		new_op test_assert_equal(user.getRealname(), "MDR2");

		new_op user.setNickname("LoL0");
		new_op test_assert_equal(user.getNickname().empty(), false);
		new_op test_assert_equal(user.getNickname(), "LoL0");

		new_op user.setUsername("LoL1");
		new_op test_assert_equal(user.getUsername().empty(), false);
		new_op test_assert_equal(user.getUsername(), "LoL1");

		new_op user.setRealname("LoL2");
		new_op test_assert_equal(user.getRealname().empty(), false);
		new_op test_assert_equal(user.getRealname(), "LoL2");

		new_op user.setAuthenticated(true);
		new_op test_assert_true(user.isAuthenticated());
		new_op user.setAuthenticated(true);
		new_op test_assert_true(user.isAuthenticated());
		new_op user.setAuthenticated(false);
		new_op test_assert_equal(user.isAuthenticated(), false);

		new_op test_assert_equal(user.kickedFromChannel(NULL), false);
		new_op test_assert_equal(user.kickedFromChannel(reinterpret_cast<data::ChannelPtr>(0xDEADBEAF)), false);

		// Message test
		{
			new_op internal::Origin msgOrigin("mdr");
			new_op std::string msgContent = "MY BIG MESSAGE";
			new_op internal::Message msg(msgOrigin, msgContent);

			new_op test_assert_true(user.sendMessage(msg));
			new_op test_assert_true(receiver.hasMessage(-1));


			new_op test_assert_true(receiver.hasMessage(-1));

			new_op internal::Message result = receiver.getLastMessage(-1);

			new_op test_assert_equal(result.getOrigin(), msgOrigin);
			new_op test_assert_equal(result.getMessage(), msgContent);
			new_op test_assert_equal(result.getChannel(), user.getNickname());
		}
	} catch (...) { test_unexpected_exception; }

	return true;
}

bool test_channel() {
	test_prelude

	try {
		MessageReceiver receiver;
		internal::Server server("", &receiver);

		const std::string name = "#nicechannel";
		data::Channel channel(name, &server);

		data::User fakeUser(-1, &server);
		data::UserPtr user = &fakeUser;

		data::User otherFakeUser(-2, &server);
		data::UserPtr otherUser = &otherFakeUser;

		// Testing default values
		new_op test_assert_equal(channel.getName(), name);
		new_op test_expect_exception(channel.isOperator(NULL), std::out_of_range);
		new_op test_assert_equal(channel.getMode(), data::Channel::CMODE_NONE);

		// Testing setters
		new_op test_expect_exception(channel.setOperator(NULL, true), std::out_of_range);
		new_op test_expect_exception(channel.setOperator(NULL, false), std::out_of_range);
		new_op test_expect_exception(channel.setOperator(reinterpret_cast<data::UserPtr>(0xDEADBEEF), true), std::out_of_range);
		new_op test_expect_exception(channel.setOperator(reinterpret_cast<data::UserPtr>(0xDEADBEEF), false), std::out_of_range);

		new_op test_assert_equal(channel.setMode(data::Channel::CMODE_OPERATOR | data::Channel::CMODE_INVITE, true), true);
		new_op test_assert_equal(channel.getMode(), data::Channel::CMODE_OPERATOR | data::Channel::CMODE_INVITE);
		new_op test_assert_equal(channel.setMode(data::Channel::CMODE_OPERATOR, false), true);
		new_op test_assert_equal(channel.getMode(), data::Channel::CMODE_INVITE);
		new_op test_assert_equal(channel.setMode(data::Channel::CMODE_INVITE, false), true);
		new_op test_assert_equal(channel.getMode(), data::Channel::CMODE_NONE);

		new_op test_assert_equal(channel.userJoin(user), true);
		new_op test_assert_equal(channel.isOperator(user), true);
		new_op test_assert_equal(channel.userJoin(user), false);
		new_op test_assert_equal(channel.isOperator(user), true);
		new_op channel.setOperator(user, false);
		new_op test_assert_equal(channel.isOperator(user), false);
		new_op channel.kickUser(user);
		new_op test_expect_exception(channel.isOperator(user), std::out_of_range);

		// Testing message

		/// No-one's here
		new_op test_assert_equal(channel.sendMessage(user, internal::Message(internal::Origin("AdMiN"), "MSG 2 OUF")), false);
		new_op test_assert_equal(channel.userJoin(user), true);

		new_op test_assert_equal(channel.sendMessage(user, internal::Message(internal::Origin("AdMiN"), "MSG 2 OUF")), true);
		new_op test_assert_equal(receiver.hasMessage(-1), false);

		new_op test_assert_equal(channel.sendMessage(otherUser, internal::Message(internal::Origin("AdMiN"), "MSG 2 OUF")), false);
		new_op test_assert_equal(channel.sendMessage(reinterpret_cast<data::UserPtr>(0xDEADBEEF), internal::Message(internal::Origin("AdMiN"), "MSG 2 OUF", true)), false);

		new_op test_assert_equal(channel.userJoin(otherUser), true);

		{
			internal::Origin orig("AdMiN (maybe)");
			std::string message = "MSG 2 OUF RESURRECTION";

			new_op test_assert_equal(channel.sendMessage(otherUser, internal::Message(orig, message)), true);
			new_op test_assert_true(receiver.hasMessage(-1));
			new_op test_assert_equal(receiver.hasMessage(-2), false);

			new_op internal::Message msg = receiver.getLastMessage(-1);
			new_op receiver.removeLastMessage(-1);
			new_op test_assert_equal(receiver.hasMessage(-1), false);
			new_op test_assert_equal(msg.getOrigin(), orig);
			new_op test_assert_equal(msg.getMessage(), message);
			new_op test_assert_equal(msg.getChannel(), channel.getName());
		}
	} catch (...) { test_unexpected_exception }
	return true;
}

bool test_server() {
	test_prelude

	try {
		MessageReceiver receiver;
		internal::Server server("PSSWD", &receiver);

		std::string channelName = "#channel";
		data::ChannelPtr channel = NULL;

		data::UserPtr user0, user1;

		new_op test_assert_equal(server.getPassword(), "PSSWD");
		new_op test_assert_equal(server.getUser(-1), NULL);
		new_op test_assert_equal(server.getUser(0xDEADBEEF), NULL);
		new_op test_assert_equal(server.getCommInterface(), &receiver);
		new_op test_assert_equal(server.getChannel("MDR"), NULL);
		new_op test_assert_equal(server.getChannel(""), NULL);
		new_op test_assert_not_equal(channel = server.getOrCreateChannel(channelName), NULL);
		new_op test_assert_equal(server.getOrCreateChannel(channelName), channel);

		new_op server.channelReclaiming(channelName);
		new_op delete channel;

		{
			new_op data::ChannelPtr fake = new data::Channel();
			new_op data::ChannelPtr new_channel = NULL;

			new_op test_assert_equal(server.getChannel(channelName), NULL);
			new_op test_assert_not_equal(new_channel = server.getOrCreateChannel(channelName), NULL);
			new_op test_assert_not_equal(new_channel, channel);
			new_op channel = new_channel;

			new_op delete fake;
		}

		new_op user0 = server.addUser(-1);
		new_op test_expect_exception(server.addUser(-1), std::runtime_error);
		new_op test_assert_equal(server.getUser(-1), user0);

		new_op user1 = server.addUser(-2);
		new_op test_expect_exception(server.addUser(-1), std::runtime_error);
		new_op test_expect_exception(server.addUser(-2), std::runtime_error);
		new_op test_assert_equal(server.getUser(-2), user1);
		new_op test_assert_not_equal(server.getUser(-2), user0);

		new_op test_assert_true(channel->userJoin(user0));
		new_op test_assert_true(channel->userJoin(user1));

		new_op test_assert_equal(server.userDisconnected(-3), false);
		new_op test_assert_equal(server.userDisconnected(0xDEADBEEF), false);
		new_op test_assert_equal(server.userDisconnected(-2), true);

		new_op test_assert_equal(server.getUser(-2), NULL);
		new_op test_expect_exception(channel->isOperator(user1), std::out_of_range);
		new_op test_assert_equal(channel->sendMessage(user0, internal::Message(internal::Origin("SCH"), "Clement >>>>> all")), true);
		new_op test_assert_equal(server.userDisconnected(-1), true);
		new_op test_assert_equal(server.getUser(-1), NULL);
		new_op test_assert_equal(server.getUser(-2), NULL);
	} catch (...) { test_unexpected_exception }
	return true;
}

struct PackedMessage {
	int fd;
	util::Optional<internal::Origin> prefix;
	std::string command;
	std::vector<std::string> parameters;
	bool lastParamExtended;

	bool equivalent(int fd, internal::Origin origin, std::string command, std::vector<std::string> parameters = std::vector<std::string>()) {
		return fd = this->fd && origin == *(this->prefix) && command == this->command && parameters == this->parameters;
	}
};

struct RealReceiver: public api::IComm {
	std::vector<PackedMessage> messages;

	RealReceiver() {}

	virtual bool stockMessage(int fd, util::Optional<internal::Origin> prefix, std::string command, std::vector<std::string> parameters, bool lastParamExtended) {
		messages.push_back((PackedMessage){.fd = fd, .prefix = prefix, .command = command, .parameters = parameters, .lastParamExtended = lastParamExtended});
		std::cout << ">> " << (prefix ? (":" + prefix->toString()) : "") << " " << command;

		for (std::size_t i = 0; i < parameters.size(); ++i) {

			std::cout << " ";
			if (lastParamExtended && i + 1 == parameters.size()) {
				std::cout << ":";
			}

			std::cout << parameters[i];
		}

		std::cout << std::endl;

		return true;
	}

	PackedMessage &at(std::size_t index) {
		return messages.at(messages.size() - 1 - index);
	}

	PackedMessage &operator[](std::size_t index) {
		return messages[messages.size() - 1 - index];
	}

	std::size_t size() {
		return messages.size();
	}
};

bool test_realcase0() {
	test_prelude

	RealReceiver receiver = RealReceiver();
	internal::Server server("POUPOU", &receiver);
	data::UserPtr user;

	std::size_t msgCount = 0;

	// HEXCHAT DROP TEST

	new_op test_assert_not_equal(user = server.addUser(2), NULL);

	new_op server.admitMessage(user->getFd(), "CAP", util::makeVector<std::string>("LS"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);

	new_op server.admitMessage(user->getFd(), "PASS", util::makeVector<std::string>("POUPOU"));
	new_op test_assert_equal(receiver.size(), msgCount);

	new_op server.admitMessage(user->getFd(), "NICK", util::makeVector<std::string>("nick"));
	new_op test_assert_equal(receiver.size(), msgCount);

	new_op server.admitMessage(user->getFd(), "USER", util::makeVector<std::string>("user", "clsaad", "localhost", "realname"));
	new_op test_assert_equal(receiver.size(), msgCount += 7);

	new_op server.admitMessage(user->getFd(), "PING", util::makeVector<std::string>("LAG1647996256762"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver.at(0).equivalent(user->getFd(), server.getHost(), "PONG", util::makeVector<std::string>(server.getHost(), "LAG1647996256762")));

	new_op server.admitMessage(user->getFd(), "PING", util::makeVector<std::string>("LAG1647996286831"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver.at(0).equivalent(user->getFd(), server.getHost(), "PONG", util::makeVector<std::string>(server.getHost(), "LAG1647996286831")));

	new_op server.admitMessage(user->getFd(), "PING", util::makeVector<std::string>("LAG1647996316899"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver.at(0).equivalent(user->getFd(), server.getHost(), "PONG", util::makeVector<std::string>(server.getHost(), "LAG1647996316899")));

	new_op server.admitMessage(user->getFd(), "PING", util::makeVector<std::string>("LAG1647996346970"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver.at(0).equivalent(user->getFd(), server.getHost(), "PONG", util::makeVector<std::string>(server.getHost(), "LAG1647996346970")));

	new_op server.admitMessage(user->getFd(), "JOIN", util::makeVector<std::string>("#test"));
	new_op test_assert_equal(receiver.size(), msgCount += 4);
	new_op test_assert_true(receiver.at(3).equivalent(user->getFd(), user->getOrigin(), "JOIN", util::makeVector<std::string>("#test")));
	new_op test_assert_true(receiver.at(2).equivalent(user->getFd(), server.getHost(), "331", util::makeVector<std::string>(user->getNickname(), "#test", "No topic is set")));
	new_op test_assert_true(receiver.at(1).equivalent(user->getFd(), server.getHost(), "353", util::makeVector<std::string>(user->getNickname(), "=", "#test", "@" + user->getNickname())));
	new_op test_assert_true(receiver.at(0).equivalent(user->getFd(), server.getHost(), "366", util::makeVector<std::string>(user->getNickname(), "#test", "End of NAMES list")));

	new_op server.admitMessage(user->getFd(), "MODE", util::makeVector<std::string>("#test"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver.at(0).equivalent(user->getFd(), server.getHost(), "324", util::makeVector<std::string>(user->getNickname(), "#test", "+")));

	new_op server.admitMessage(user->getFd(), "WHO", util::makeVector<std::string>("#test"));
	new_op test_assert_equal(receiver.size(), msgCount += 2);
	new_op test_assert_true(receiver.at(1).equivalent(user->getFd(), server.getHost(), "352",
		util::makeVector<std::string>(user->getNickname(), "#test", "user", "localhost", server.getHost(), user->getNickname(), "H@", "0 " + user->getRealname())));
	new_op test_assert_true(receiver.at(0).equivalent(user->getFd(), server.getHost(), "315", util::makeVector<std::string>(user->getNickname(), "#test", "End of WHO list")));

	new_op server.admitMessage(user->getFd(), "PING", util::makeVector<std::string>("LAG1647996377045"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver.at(0).equivalent(user->getFd(), server.getHost(), "PONG", util::makeVector<std::string>(server.getHost(), "LAG1647996377045")));

	new_op server.admitMessage(user->getFd(), "PRIVMSG", util::makeVector<std::string>("#test", "Saluuuut"));
	new_op test_assert_equal(receiver.size(), msgCount);

	new_op server.admitMessage(user->getFd(), "PING", util::makeVector<std::string>("LAG1647996407118"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver.at(0).equivalent(user->getFd(), server.getHost(), "PONG", util::makeVector<std::string>(server.getHost(), "LAG1647996407118")));

	new_op server.admitMessage(user->getFd(), "PART", util::makeVector<std::string>("#test", "Leaving"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver.at(0).equivalent(user->getFd(), user->getOrigin(), "PART", util::makeVector<std::string>("#test", "Leaving")));

	new_op test_assert_equal(server.getChannel("#test"), NULL);

	// ERROR TESTING

	/// JOIN
	new_op server.admitMessage(user->getFd(), "JOIN", util::makeVector<std::string>("invalid"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver[0].equivalent(user->getFd(), server.getHost(), "403", util::makeVector<std::string>(user->getNickname(), "invalid", "No such channel")));

	new_op server.admitMessage(user->getFd(), "JOIN", util::makeVector<std::string>("#"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver[0].equivalent(user->getFd(), server.getHost(), "403", util::makeVector<std::string>(user->getNickname(), "#", "No such channel")));

	// TODO: MORE JOIN TESTS

	/// PRIVMSG/NOTICE
	new_op server.admitMessage(user->getFd(), "PRIVMSG", util::makeVector<std::string>("#test", "Saluuuut"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver[0].equivalent(user->getFd(), server.getHost(), "401", util::makeVector<std::string>(user->getNickname(), "#test", "No such nick/channel")));

	new_op server.admitMessage(user->getFd(), "NOTICE", util::makeVector<std::string>("#test", "Saluuuut"));
	new_op test_assert_equal(receiver.size(), msgCount);

	new_op server.admitMessage(user->getFd(), "PRIVMSG", util::makeVector<std::string>("nouserlmao", "Saluuuut"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver[0].equivalent(user->getFd(), server.getHost(), "401", util::makeVector<std::string>(user->getNickname(), "nouserlmao", "No such nick/channel")));

	new_op server.admitMessage(user->getFd(), "NOTICE", util::makeVector<std::string>("nouserlmao", "Saluuuut"));
	new_op test_assert_equal(receiver.size(), msgCount);

	new_op server.admitMessage(user->getFd(), "PRIVMSG", util::makeVector<std::string>("nouserlmao"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver[0].equivalent(user->getFd(), server.getHost(), "412", util::makeVector<std::string>(user->getNickname(), "No text to send")));

	new_op server.admitMessage(user->getFd(), "NOTICE", util::makeVector<std::string>("nouserlmao"));
	new_op test_assert_equal(receiver.size(), msgCount);

	new_op server.admitMessage(user->getFd(), "PRIVMSG", util::makeVector<std::string>("nouserlmao", ""));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver[0].equivalent(user->getFd(), server.getHost(), "412", util::makeVector<std::string>(user->getNickname(), "No text to send")));

	new_op server.admitMessage(user->getFd(), "NOTICE", util::makeVector<std::string>("nouserlmao", ""));
	new_op test_assert_equal(receiver.size(), msgCount);

	new_op server.admitMessage(user->getFd(), "PRIVMSG", util::makeVector<std::string>(""));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver[0].equivalent(user->getFd(), server.getHost(), "411", util::makeVector<std::string>(user->getNickname(), "No recipient given (PRIVMSG)")));

	new_op server.admitMessage(user->getFd(), "NOTICE", util::makeVector<std::string>(""));
	new_op test_assert_equal(receiver.size(), msgCount);

	new_op server.admitMessage(user->getFd(), "PRIVMSG", util::makeVector<std::string>());
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver[0].equivalent(user->getFd(), server.getHost(), "411", util::makeVector<std::string>(user->getNickname(), "No recipient given (PRIVMSG)")));

	new_op server.admitMessage(user->getFd(), "NOTICE", util::makeVector<std::string>());
	new_op test_assert_equal(receiver.size(), msgCount);

	// TODO: add more PRIVMSG/NOTICE test with channels

	/// PING/PONG

	new_op server.admitMessage(user->getFd(), "PING", util::makeVector<std::string>());
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver[0].equivalent(user->getFd(), server.getHost(), "409", util::makeVector<std::string>(user->getNickname(), "No origin specified")));

	new_op server.admitMessage(user->getFd(), "PING", util::makeVector<std::string>("TESTPING0", ""));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver[0].equivalent(user->getFd(), server.getHost(), "402", util::makeVector<std::string>(user->getNickname(), "", "No such server")));

	new_op server.admitMessage(user->getFd(), "PING", util::makeVector<std::string>("TESTPING1", "not.our.server"));
	new_op test_assert_equal(receiver.size(), msgCount += 1);
	new_op test_assert_true(receiver[0].equivalent(user->getFd(), server.getHost(), "402", util::makeVector<std::string>(user->getNickname(), "not.our.server", "No such server")));

	return true;
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

#if !(__has_feature(address_sanitizer))
	std::cout << "AddressSanitizer disabled, handling signal manually" << std::endl;
	std::signal(SIGSEGV, handle_sigs);
#else
	std::cout << "AddressSanitizer enabled" << std::endl;
#endif

	EXEC_TEST(test_user)
	EXEC_TEST(test_channel)
	EXEC_TEST(test_server)
	EXEC_TEST(test_realcase0)

	return 0;
}
