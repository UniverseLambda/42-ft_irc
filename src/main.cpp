#include <emma/Parsing.hpp>
#include <emma/Msg_manager.hpp>


int	main(int argc, char **argv){
	msg_manager		manager;

	if (argc != 2){
		std::cout << "Error : wrong argument(s)\n";
		exit(EXIT_FAILURE);
	}
	manager.set_connection(argv[1]);
	manager.connections_manager();
	return 0;
}
