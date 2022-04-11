#include <network/Parsing.hpp>
#include <network/Msg_manager.hpp>
#include <iostream>
#include <vector>

void	msg_parser(std::string msg, int fd, internal::ServerPtr server){
	size_t						i = 0;
	size_t						start = 0;
	std::string					cmd;
	std::vector<std::string>	params;
	size_t						len = msg.size();

	if (len <= 0)
		return ;
	while (i < len){
		while (i < len && msg[i] == ' ')
			i++;
		start = i;
		if (i < len && msg[i] == ':' && cmd.size() > 0){
			start += 1;
			while (i < len)
				i++;
		}
		else {
			while (i < len && msg[i] != ' ')
				i++;
		}
		if (i > start && cmd.size() == 0)
			cmd = msg.substr(start, i - start);
		else if (i > start)
			params.push_back(msg.substr(start, i - start));
	}
	server->admitMessage(fd, cmd, params);
	params.clear();
}

void find_msg(msg_manager &MM, int fd, char *msg, internal::ServerPtr server){
	size_t								i = 0;
	std::string							tmp(msg);
	std::map<int, content >::iterator	it = MM.received_msg.find(fd);
	size_t								size = tmp.size();
	size_t								start = 0;

	while (i < size){
		if ((tmp[i] == '\r' && tmp[i + 1] && tmp[i + 1] == '\n')
			|| (tmp[i] == '\n' && it != MM.received_msg.end() && it->second.r)){
			if (it == MM.received_msg.end()){
				msg_parser(tmp.substr(start, i - start), fd, server);
				if (tmp[i] && tmp[i] == '\r')
					i++;
				if (tmp[i] && tmp[i] == '\n')
					i++;
				start = i;
				continue ;
			}
			it->second.buff += tmp.substr(start, i);
			msg_parser(it->second.buff, fd, server);
			MM.received_msg.erase(fd);
			it = MM.received_msg.find(fd);
			if (tmp[i] && tmp[i] == '\r')
				i++;
			if (tmp[i] && tmp[i] == '\n')
				i++;
			start = i;
			continue ;
		}
		else
			i++;
	}
	if (i > start && size > 0){
		if (it == MM.received_msg.end()){
			MM.received_msg.insert(std::make_pair(fd, content()));}
		it = MM.received_msg.find(fd);
		it->second.buff += tmp.substr(start, size - start);
		if (tmp[size - 1] == '\r')
			it->second.r = true;
		else
			it->second.r = false;
	}
}
