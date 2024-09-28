#include "Server.hpp"

static bool isValidPort(std::string port)
{
	return (port.find_first_not_of("0123456789") == std::string::npos &&
			std::atoi(port.c_str()) >= 1024 && std::atoi(port.c_str()) <= 65535);
}

int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <port_number> <password>" << std::endl;
		return 1;
	}
	std::cout << "---- SERVER ----" << std::endl;
	try {
		Server ser;
		std::signal(SIGINT, Server::signalHandler);  //-> catch the signal (ctrl + c)
		std::signal(SIGQUIT, Server::signalHandler); //-> catch the signal (ctrl + \)
		if (!isValidPort(argv[1])) {
			std::cerr << "Invalid Port number range(1024~65535)" << std::endl;
			return 1;
		} else if (!*argv[2] || std::strlen(argv[2]) < 3 || std::strlen(argv[2]) > 20) {
			std::cerr
					<< "Invalid password. Password must be between 3 to 20 characters."
					<< std::endl;
			return 1;
		}
		ser.serverInit(std::atoi(argv[1]), argv[2]); //-> initialize the server
	} catch (const std::bad_alloc& ba) {
		std::cerr << "The Server will close due to Low Memory: " << ba.what() << std::endl;
		return 1;
	} catch (const std::exception& e) {
		std::cerr << "The Server will close due to: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "The Server Closed!" << std::endl;
	return 0;
}
