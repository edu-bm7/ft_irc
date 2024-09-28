#include "Server.hpp"
#include "helper_functions.hpp"

void Server::partChannel(Client* client, const std::string& channelName,
						 const std::string& reason)
{
	Channel* channel = getChannel(channelName);
	if (!channel) {
		sendError(403, client->getNickname(), channelName, client->getFd(), " :No such channel\r\n");
		return;
	}

	// Check if the client is a member of the channel
	if (!channel->getClientInChannel(client->getNickname())) {
		sendError(442, client->getNickname(), channelName, client->getFd(), " :You're not on that channel\n\r");
		return;
	}

	// Remove the client from the channel
	channel->removeClient(client->getFd());

	// Construct the PART message
	std::string partMsg = ":" + client->getFullMask() + " PART " + channelName;
	if (!reason.empty()) {
		partMsg += " :" + reason;
	}
	partMsg += "\r\n";

	// Send the PART message to the channel
	channel->broadcastMessage(partMsg);
	sendResponse(partMsg, client->getFd());

	// If the client was an operator, remove them from the operator list
	if (channel->getOperator(client->getFd())) {
		channel->removeOperator(client->getFd());
	}

	// If the channel is now empty, delete it
	if (channel->getClientsNumber() == 0) {
		for (size_t i = 0; i < mChannels.size(); ++i) {
			if (mChannels[i].getChannelName() == channelName) {
				mChannels.erase(mChannels.begin() + i);
				break;
			}
		}
	}
	client->removeChannelInvite(channelName);
}

void Server::partCommand(std::string& cmd, int fd)
{
	Client* client = getClientFromFd(fd);
	if (!client) {
		return;
	}

	// Trim leading and trailing whitespace
	std::string cmd_line = trim(cmd);

	// Remove "PART" from the command
	const std::string part_cmd = "PART";
	size_t pos = cmd.find(part_cmd);
	if (pos != 0) {
		return;
	}
	pos += part_cmd.length();

	// Skip whitespace
	while (pos < cmd.length() && isspace(cmd[pos])) {
		pos++;
	}

	if (pos >= cmd.length()) {
		// No parameters provided
		sendError(461, client->getNickname(), client->getFd(), " :Not enough parameters\r\t");
		return;
	}

	// Extract the parameters
	std::string params = cmd.substr(pos);

	// Split the parameters into channels and optional reason
	std::string channels_part;
	std::string reason;
	size_t colon_pos = params.find(" :");
	if (colon_pos != std::string::npos) {
		channels_part = params.substr(0, colon_pos);
		reason = params.substr(colon_pos + 2); // Skip " :"
	} else {
		channels_part = params;
		reason = ""; // No reason provided
	}

	channels_part = trim(channels_part);

	// Split channels by commas
	std::vector< std::string > channels = splitByComma(channels_part);

	// Process each channel
	for (size_t i = 0; i < channels.size(); ++i) {
		std::string channelName = channels[i];
		partChannel(client, channelName, reason);
	}
}
