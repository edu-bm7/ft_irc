#include "Server.hpp"
#include "helper_functions.hpp"

void Server::whoisCommand(std::string& cmd, int fd)
{
	Client* client = getClientFromFd(fd);
	if (!client) {
		return;
	}

	std::string cmd_line = trim(cmd);

	const std::string whois_cmd = "WHOIS";
	size_t pos = cmd.find(whois_cmd);
	if (pos != 0) {
		return;
	}

	pos += whois_cmd.length();

	// Skip whitespace
	while (pos < cmd.length() && isspace(cmd[pos])) {
		pos++;
	}

	if (pos >= cmd.length()) {
		// No parameters provided
		sendError(461, client->getNickname(), client->getFd(), " :Not enough parameters\r\t");
		return;
	}

	// Extract the rest of the command
	std::string params = cmd.substr(pos);
	params = trim(params);

	std::vector< std::string > targetList = splitByComma(params);
	for (size_t i = 0; i < targetList.size(); ++i) {
		Client* target = getClientFromNickname(targetList[i]);
		if (!target) {
			sendResponse(ERR_NOSUCHNICK(client->getNickname(), targetList[i]), fd);
			continue;
		}

		sendResponse(RPL_WHOISUSER(client->getNickname(),
								   target->getNickname(),
								   target->getUsername()),
					 fd);
		sendResponse(RPL_WHOISSERVER(client->getNickname(),
									 target->getNickname(),
									 target->getUsername()),
					 fd);
		if (target->getIsOperator()) {
			sendResponse(RPL_WHOISOPERATOR(client->getNickname(), target->getNickname()),
						 fd);
		}
		sendResponse(RPL_WHOISIDLE(client->getNickname(),
								   target->getNickname(),
								   std::to_string(target->getIdleTime())),
					 fd);
		sendResponse(RPL_WHOISCHANNELS(client->getNickname(),
									   target->getNickname(),
									   "*"),
					 fd);
		sendResponse(RPL_ENDOFWHOIS(client->getNickname(), target->getNickname()), fd);
	}
}
