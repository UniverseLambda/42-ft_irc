#include <emma/Msg_manager.hpp>

msg_manager::msg_manager(){
	listen_fd = 0;
	new_socket = 0;
	server = internal::Server("POUPOU", this);
}

bool msg_manager::sendMessage(int fd, util::Optional<internal::Origin> prefix, std::string command, std::vector<std::string> parameters, bool lastParamExtended){
	std::string	res;
	size_t		params_nb = parameters.size();
	size_t		sent = 0;

	if (!(poll_fds[fd].revents & POLLOUT))
		return 0;
	if (prefix)
		res += prefix->toString() + ' ';
	res += command + ' ';
	for (size_t i = 0; i < params_nb ; i++){
		if (lastParamExtended && i == (params_nb - 1))
			res += ':';
		if (i == params_nb - 1)
			res += parameters[i];
		else
			res += parameters[i] + ' ';
	}
	while ((sent = write(fd, res, res.size())) != res.size()){
		std::map<int, content >::iterator	it = to_send.find(fd);
		if (it == to_send.end()){
			to_send.insert(std::make_pair(fd, content()));
			it = to_send.find(fd);
		}
		it->second.buff += res.substr(sent, res.size() - sent);
		res = it->second.buff;
	}
	return 1;
} 

void	msg_manager::set_connection(char *arg){

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		std::cout << "Error\n";
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(atoi(arg)); // from little endian to big endian

	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) == -1){
		std::cout << "Error, failed to bind\n";
		exit(EXIT_FAILURE);
	}
	if (listen(listen_fd, 100) < 0){
		std::cout << "Error, failed to listen on socket\n";
		exit(EXIT_FAILURE);
	}
	poll_fds.push_back((pollfd){.fd = listen_fd, .events = POLLIN|POLLOUT});
}

void	msg_manager::connections_manager(){
	int	new_co;

	while (42){													// check for potential read/write/accept
		new_co = poll(poll_fds.data(), poll_fds.size(), 100);
		if (new_co < 0){
			std::cout << "Poll failed\n";
			break;
		}
		else if (new_co == 0)
			continue;
		else if (poll_fds[0].revents & POLLIN){
			new_socket = accept(listen_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
			fcntl(new_socket, F_SETFL, O_NONBLOCK);
			if (new_socket < 0){
				std::cout << "Error, failed to grab connection\n";
				exit(EXIT_FAILURE);
			}
			poll_fds.push_back((pollfd){.fd = new_socket, .events = POLLIN|POLLOUT, .revents = 0});
		}
		std::vector<pollfd>::iterator it = poll_fds.begin() + 1;
		while (it != poll_fds.end()){
			if (it->revents & POLLIN){
				int result = 1;
				memset(buffer, 0, 4097);
				result = read(it->fd, buffer, 4096);
				std::string str;
				find_msg(&received_msg, it->fd, buffer, &server);
				if (result < 0)
					std::cout << "couldn't read from socket\n";
				else if (result == 0){
					close(it->fd);
					server.userDisconnected(it->fd);
					it = poll_fds.erase(it);
					continue;
				}
			}
			it++;
		}
	}
}