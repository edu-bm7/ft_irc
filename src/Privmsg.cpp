#include "Server.hpp"
#include "helper_functions.hpp"

void Server::privmsgCommand(std::string& cmd, int fd)
{
	// Check if the target user exists
	Client* sender = getClientFromFd(fd);
	if (!sender) {
		return;
	}

	// Trim leading and trailing whitespace
	std::string cmd_line = trim(cmd);

	// Remove "PRIVMSG" from the command
	const std::string privmsg_cmd = "PRIVMSG";
	size_t pos = cmd.find(privmsg_cmd);
	if (pos != 0) {
		return;
	}

	pos += privmsg_cmd.length();

	// Skip whitespace
	while (pos < cmd.length() && isspace(cmd[pos])) {
		pos++;
	}

	if (pos >= cmd.length()) {
		sendError(411, sender->getNickname(),
				  sender->getFd(), " :No recipient given PRIVMSG\r\t");
		return;
	}

	// Extract the parameters
	std::string params = cmd.substr(pos);
	params = trim(params);

	// Split params into targets and message
	size_t colon_pos = params.find(" :");
	std::string targets_part;
	std::string message;

	if (colon_pos != std::string::npos) {
		targets_part = params.substr(0, colon_pos);
		message = params.substr(colon_pos + 2);
	} else {
		targets_part = params;
		message = "";
	}

	targets_part = trim(targets_part);

	// No receivers was sent
	if (targets_part.empty()) {
		sendError(411, sender->getNickname(),
				  sender->getFd(), " :No recipient given PRIVMSG\r\t");
		return;
	}

	std::vector< std::string > targets = splitByComma(targets_part);

	if (message.empty()) {
		sendError(412, sender->getNickname(),
				  sender->getFd(), " :No text to send\r\n");
		return;
	}

	// if there are more then 10 recipients set ERR_TOOMANYTARGETS to not convolute
	// the server
	if (targets.size() > 10) {
		sendError(407, sender->getNickname(),
				  sender->getFd(), " :Too many recipients\r\n");
		return;
	}

	for (size_t i = 0; i < targets.size(); ++i) {
		std::string target = targets[i];
		if (target.empty()) {
			continue;
		}

		if (isValidChannelName(target)) {
			// Target is a channel
			Channel* channel = getChannel(target);

			if (!channel) {
				// No such nick; send ERR_NOSUCHNICK (401)
				sendError(401, target,
						  sender->getFd(), " :No such nick/channel\r\n");
				continue;
			}

			if (!channel->getClientInChannel(sender->getNickname())) {
				// You're not on that channel; send ERR_CANNOTSENDTOCHAN (404)
				sendError(404, sender->getNickname(), target,
						  sender->getFd(), " :Cannot send to channel\r\n");
				continue;
			}

			// Broadcast the message to all channel members except sender
			std::string privmsg = ":" + sender->getFullMask() + " PRIVMSG " + target + " :" + message + "\r\n";
			channel->broadcastMessage(privmsg, sender->getFd());
		} else {
			// Target is a user
			Client* targetClient = getClientFromNickname(target);

			if (!targetClient) {
				// No suck nick; send ERR_NOSUCHNICK (401)
				sendError(401, target,
						  sender->getFd(), " :No such nick/channel\r\n");
				continue;
			}

			// Send the private message to the target user
			std::string privmsg = ":" + sender->getFullMask() + " PRIVMSG " + target + " :" + message + "\r\n";
			sendResponse(privmsg, targetClient->getFd());
		}
	}
}
