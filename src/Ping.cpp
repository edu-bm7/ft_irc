#include "Server.hpp"
#include "helper_functions.hpp"

void Server::pingCommand(std::string& cmd, int fd)
{
	Client* client = getClientFromFd(fd);
	if (!client) {
		return;
	}

	// Trim leading and trailing whitespace
	std::string cmd_line = trim(cmd);

	// Remove "PING" from the command
	const std::string ping_cmd = "PING";
	size_t pos = cmd.find(ping_cmd);
	if (pos != 0) {
		return;
	}

	pos += ping_cmd.length();

	// Skip whitespace
	while (pos < cmd.length() && isspace(cmd[pos])) {
		pos++;
	}

	if (pos >= cmd.length()) {
		// No parameters provided
		sendResponse(ERR_NOORIGIN(client->getNickname()), fd);
		return;
	}

	std::string params = cmd.substr(pos);
	params = trim(params);
	std::string target;

	size_t colon_pos = params.find(" :");
	if (colon_pos != std::string::npos) {
		target = params.substr(colon_pos + 2);
	} else {
		target = params;
	}
	if (target != "localhost" && target != "127.0.0.1") {
		sendResponse(ERR_NOSUCHSERVER(client->getNickname(), target), fd);
		return;
	}
	sendResponse(RPL_PONG(target), fd);
}
