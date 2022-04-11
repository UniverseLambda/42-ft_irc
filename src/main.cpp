#include <network/Parsing.hpp>
#include <network/Msg_manager.hpp>


int	main(int argc, char **argv){
	if (argc != 3){
		std::cerr << "Error : wrong argument(s)\n";
		exit(EXIT_FAILURE);
	}

	msg_manager		manager(argv[2]);
	manager.set_connection(argv[1]);
	manager.connections_manager();
	return 0;
}
