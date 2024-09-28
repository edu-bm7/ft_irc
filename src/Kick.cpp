#include "Server.hpp"
#include "helper_functions.hpp"

static std::vector< std::string > splitKick(std::string cmd_line)
{
	std::string cmd = trim(cmd_line);
	std::vector< std::string > vec;
	const std::string kick_cmd = "KICK";

	size_t pos = 0;
	// Ensure the command starts with "KICK"
	if (cmd.substr(0, kick_cmd.length()) != kick_cmd) {
		return vec;
	}

	pos += kick_cmd.length();
	// Skip whitespace after "KICK"
	while (pos < cmd.length() && std::isspace(cmd[pos])) {
		pos++;
	}

	if (pos >= cmd.length()) {
		return vec; // No parameters provided
	}

	// Extract the channel name
	size_t channel_start = pos;
	while (pos < cmd.length() && !std::isspace(cmd[pos])) {
		pos++;
	}
	std::string channel = cmd.substr(channel_start, pos - channel_start);

	// Skip whitespace before the username
	while (pos < cmd.length() && std::isspace(cmd[pos])) {
		pos++;
	}

	if (pos >= cmd.length()) {
		return vec; // No username provided
	}

	// Extract the username
	size_t user_start = pos;
	while (pos < cmd.length() && !std::isspace(cmd[pos])) {
		pos++;
	}
	std::string username = cmd.substr(user_start, pos - user_start);

	// Skip whitespace before the reason (if any)
	while (pos < cmd.length() && std::isspace(cmd[pos])) {
		pos++;
	}

	// Populate the vector
	vec.push_back(kick_cmd); // vec[0]: Command "KICK"
	vec.push_back(channel);  // vec[1]: Channel name
	vec.push_back(username); // vec[2]: Username

	// Extract the reason (if any)
	std::string reason;
	if (pos < cmd.length()) {
		if (cmd[pos] == ':') {
			pos++; // Skip the colon
		}
		reason = cmd.substr(pos);
		vec.push_back(reason); // vec[3]: Reason
	}
	return vec;
}

void Server::kickCommand(std::string& cmd, int fd)
{
	std::vector< std::string > params = splitKick(cmd);

	Client* client = getClientFromFd(fd);
	if (!client) {
		return;
	}

	if (params.size() < 3) {
		sendError(461, client->getNickname(), client->getFd(), " :Not enough parameters\r\n");
		return;
	}

	std::string channelName = params[1];
	std::string targetNick = params[2];
	std::string reason = (params.size() > 3) ? params[3] : "";


	Channel* channel = getChannel(channelName);
	if (!channel) {
		sendError(403, client->getNickname(), channelName, client->getFd(), " :No such channel\r\n");
		return;
	}

	if (!channel->returnClientInChannel(client->getNickname())) {
		sendError(442, client->getNickname(), channelName, client->getFd(), " :You're not on that channel\n\r");
		return;
	}

	if (!channel->getOperator(fd)) {
		sendError(482, client->getNickname(), channelName, client->getFd(), " :You're not channel operator\r\n");
		return;
	}

	Client* targetClient = getClientFromNickname(targetNick);
	if (!targetClient) {
		sendError(401, targetClient->getNickname(), channelName, client->getFd(), " :No such nick/channel\r\n");
		return;
	}

	if (!channel->returnClientInChannel(targetClient->getNickname())) {
		sendError(442, targetClient->getNickname(), channelName, client->getFd(), " :They aren't on that channel\r\n");
		return;
	}

	// Remove the user
	if (channel->getOperator(targetClient->getFd())) {
		channel->removeOperator(targetClient->getFd());
	} else {
		channel->removeClient(targetClient->getFd());
	}

	// Notify all channel members
	std::string kickMessage = ":" + client->getFullMask() + " KICK " + channelName + " " + targetNick;
	if (!reason.empty()) {
		kickMessage += " :" + reason;
	}

	channel->broadcastMessage(kickMessage);
}
