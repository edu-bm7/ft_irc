#include "Server.hpp"
#include "helper_functions.hpp"

void Server::joinChannel(Client* client, std::string& channelName, std::string& key)
{
	// Validate the channel name
	if (!isValidChannelName(channelName)) {
		sendError(403, client->getNickname(), channelName, client->getFd(), " :No such channel\r\n");
		return;
	}

	// Check if the channel exists
	Channel* channel = getChannel(channelName);
	if (!channel) {
		// Channel doesn't exist; create it
		Channel newChannel;
		newChannel.setChannelName(channelName);
		newChannel.addOperator(*client);
		newChannel.setCreationTime();
		mChannels.push_back(newChannel);
		// Send JOIN message to the client and other channel members
		sendResponse(RPL_JOINMSG(client->getFullMask(), client->getIpAddr(), channelName) +
							 RPL_NAMREPLY(client->getNickname(), channelName, newChannel.getChannelClientList()) +
							 RPL_ENDOFNAMES(client->getNickname(), channelName),
					 client->getFd());
		return;
	}

	// Check if the client is already in the channel
	if (channel->getClientInChannel(client->getNickname())) {
		// Client is already in the channel; do nothing
		return;
	}

	// Check if the channel requires a key
	if (channel->getChannelProtected()) {
		if (channel->getChannelKey() != key) {
			sendError(475, client->getNickname(), channelName,
					  client->getFd(), " :Cannot join channel (+k) - bad key\r\t");
			return;
		}
	} else if (!key.empty()) {
		// If the channel doesn't have a key but the client provided one, ignore it
	}

	if (channel->getInviteOnly()) {
		if (!client->getIsInvitedToChannel(channelName)) {
			sendError(473, client->getNickname(), channelName,
					  client->getFd(), " :Cannot join channel (+i)\r\n");
			return;
		}
	}

	if (channel->getChannelLimitted()) {
		if (channel->getClientsNumber() >= channel->getChannelLimit()) {
			sendError(471, client->getNickname(), channelName,
					  client->getFd(), " :Cannot join channel (+l)\r\n");
			return;
		}
	}

	// Add the client to the channel
	channel->addClient(*client);
	client->removeChannelInvite(channel->getChannelName());


	// Send channel topic if it exists
	if (!channel->getTopicName().empty()) {
		sendResponse(RPL_JOINMSG(client->getFullMask(),
								 client->getIpAddr(), channelName) +
							 RPL_TOPICIS(client->getNickname(), channelName, channel->getTopicName()) +
							 RPL_NAMREPLY(client->getNickname(), channelName, channel->getChannelClientList()) +
							 RPL_ENDOFNAMES(client->getNickname(), channelName),
					 client->getFd());
	} else {
		sendResponse(RPL_JOINMSG(client->getFullMask(), client->getIpAddr(), channelName) +
							 RPL_NAMREPLY(client->getNickname(), channelName, channel->getChannelClientList()) +
							 RPL_ENDOFNAMES(client->getNickname(), channelName),
					 client->getFd());
	}

	// Send JOIN message to the client and other channel members
	channel->broadcastMessage(RPL_JOINMSG(client->getFullMask(),
										  client->getIpAddr(), channelName),
							  client->getFd());
}

void Server::joinCommand(std::string& cmd, int fd)
{
	Client* client = getClientFromFd(fd);
	if (!client) {
		return;
	}

	// Trim leading and trailing whitespace
	std::string cmd_line = trim(cmd);

	// Remove "JOIN" from the command
	const std::string join_cmd = "JOIN";
	size_t pos = cmd.find(join_cmd);
	if (pos != 0) {
		return;
	}

	pos += join_cmd.length();

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

	// Handle the special case of "JOIN 0"
	if (params == "0") {
		// Part the client from all channels
		partAllChannels(fd);
		return;
	}

	// Split the parameters into channel and optional key
	std::string channelName;
	std::string key;

	size_t space_pos = params.find(' ');
	if (space_pos != std::string::npos) {
		// There is a key
		channelName = params.substr(0, space_pos);
		key = params.substr(space_pos + 1);
		key = trim(key); // Remove any extra whitespace
	} else {
		// No key provided
		channelName = params;
	}

	// Proceed to join the channel
	joinChannel(client, channelName, key);
}
