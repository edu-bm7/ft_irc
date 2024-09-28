#include "Server.hpp"
#include "helper_functions.hpp"

static std::string splitQuit(std::string cmd_line)
{
	std::string cmd = trim(cmd_line);
	const std::string quit_cmd = "QUIT";
	// Skip over "QUIT" and any following spaces
	size_t pos = quit_cmd.length();
	while (pos < cmd.length() && isspace(cmd[pos])) {
		pos++;
	}

	// Check if there's a parameter
	if (pos >= cmd.length()) {
		// No parameter provided
		return "";
	}

	// Check if the parameter starts with a colon
	if (cmd[pos] == ':') {
		// Extract the rest of the line as the quit message
		return cmd.substr(pos + 1);
	} else {
		// No colon but additional text; extract as is
		return cmd.substr(pos);
	}
}

void Server::quitCommand(std::string& cmd, int fd)
{
	std::string reason = splitQuit(cmd);
	removeClientFromChannels(fd, reason);
	std::cout << RED << "Client <" << fd << "> Disconnected" << WHITE << std::endl;
	removeClient(fd);
	removeFd(fd);
	close(fd);
	fd = -1;
}
