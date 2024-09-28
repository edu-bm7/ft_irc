#include "Server.hpp"
#include "helper_functions.hpp"

void Server::topicCommand(std::string& cmd, int fd)
{
	Client* client = getClientFromFd(fd);
	if (!client) {
		return;
	}

	// Trim leading and trailing whitespace
	std::string cmd_line = trim(cmd);

	// Remove "TOPIC" from the command
	const std::string join_cmd = "TOPIC";
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

	// Split the parameters into channel and optional key
	std::string channelName;
	std::string topic = "";
	bool trailing_param = false;
	size_t space_pos = params.find(' ');
	if (space_pos != std::string::npos) {
		// There is a topic
		channelName = params.substr(0, space_pos);
		if (params[space_pos + 1] == ':') {
			trailing_param = true;
			space_pos++;
		}
		topic = params.substr(space_pos + 1);
		topic = trim(topic); // Remove any extra whitespace
	} else {
		// No topic provided
		channelName = params;
	}
	Channel* channel = getChannel(channelName);
	if (!channel) {
		sendError(403, channelName, fd, " :No such channel\r\n");
		return;
	}
	if (!trailing_param && topic.empty()) {
		// Returns topic name for this channel
		std::string currentTopic = ": 332 " + client->getNickname() + channelName + " :" + channel->getTopicName() + "\r\n";
		sendResponse(currentTopic, client->getFd());
		return;
	}
	if (channel->getOperator(client->getFd()) || !channel->getTopicRestriction()) {
		channel->setTopicName(topic);
		std::string topicMsg = ":" + client->getFullMask() + " TOPIC " + channelName + " :" + topic + "\r\n";
		channel->broadcastMessage(topicMsg);
		return;
	}

	if (!channel->getOperator(client->getFd())) {
		sendError(482, client->getNickname(), channelName, client->getFd(), " :You're not channel operator\r\n");
	}
}
