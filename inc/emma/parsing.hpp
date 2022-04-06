#ifndef SERVER_HPP
# define SERVER_HPP

#include <sstream>
#include <sys/socket.h>
#include <functional>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <string>
#include <sys/poll.h>
#include <vector>
#include <internal/Server.hpp>

struct	content{
	bool		r;
	std::string	buff;
};

class msg_manager;

void	msg_parser(std::string msg, int fd, internal::ServerPtr server);
void 	find_msg(msg_manager &MM, int fd, char *msg, internal::ServerPtr server);


#endif