#ifndef MSG_MANAGER_HPP
# define MSG_MANAGER_HPP

#include <network/Parsing.hpp>
#include <internal/Server.hpp>
#include <cstdlib>
#include <netinet/in.h>
#include <algorithm>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <iostream>
#include <api/IComm.hpp>
#include <internal/Server.hpp>
#include <sys/poll.h>
#include <internal/Origin.hpp>
#include <util/Util.hpp>
#include <sys/socket.h>

class	msg_manager: public api::IComm{
	private :
	int								listen_fd, new_socket;
	sockaddr_in						address;
	char							buffer[4097];
	std::map<int, struct content>	to_send;
	std::vector<pollfd>				poll_fds;
	int								addrlen;
	internal::Server 				server;
	public :
	std::map<int, struct content>	received_msg;
	msg_manager(char *mdp);
	void	set_connection(char *arg);
	void	connections_manager();
	bool	sendMessage(int fd);
	bool	stockMessage(int fd, util::Optional<internal::Origin> prefix, std::string command, std::vector<std::string> parameters, bool lastParamExtended);

};


#endif