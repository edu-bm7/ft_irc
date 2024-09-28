#include "Server.hpp"
#include "helper_functions.hpp"

void Server::broadcastModeChange(Channel* channel,
								 Client* client,
								 char mode,
								 bool adding,
								 const std::string& param)
{
	std::string modeChange = ":" + client->getFullMask() + " MODE " + channel->getChannelName() + " " + (adding ? "+" : "-") + mode;
	if (!param.empty()) {
		modeChange += " " + param;
	}
	modeChange += "\r\n";

	channel->broadcastMessage(modeChange);
}

void Server::handleParamMode(Channel* channel, Client* client,
							 char mode, bool adding,
							 std::vector< std::string >& modeParams,
							 size_t& paramIndex)
{
	if (adding) {
		if (paramIndex >= modeParams.size()) {
			sendError(461, client->getNickname(), client->getFd(), " :Not enough parameters\r\t");
			return;
		}
	}

	switch (mode) {
		case 'k': // Channel key
			if (adding) {
				std::string key = modeParams[paramIndex++];
				channel->setChannelProtected(true);
				channel->setKey(key);
				broadcastModeChange(channel, client, mode, adding, key);
			} else {
				channel->setChannelProtected(false);
				channel->setKey("");
				broadcastModeChange(channel, client, mode, adding);
			}
			break;
		case 'l': // User limit
			if (adding) {
				std::string limitStr = modeParams[paramIndex++];
				int limit = atoi(limitStr.c_str());
				channel->setChannelLimited(true);
				channel->setLimit(limit);
				broadcastModeChange(channel, client, mode, adding, limitStr);
			} else {
				channel->setChannelLimited(false);
				channel->setLimit(0);
				broadcastModeChange(channel, client, mode, adding);
			}
			break;
		case 'o': // Operator privilege
		{
			std::string targetNick = modeParams[paramIndex++];
			Client* targetClient = getClientFromNickname(targetNick);
			if (!targetClient || !channel->getClientInChannel(targetClient->getNickname())) {
				sendError(441, targetNick, channel->getChannelName(), client->getFd(), " :They aren't on that channel");
				return;
			}
			if (adding) {
				channel->addOperator(*targetClient);
				channel->removeClient(targetClient->getFd());
			} else {
				channel->removeOperator(targetClient->getFd());
				channel->addClient(*targetClient);
			}
			broadcastModeChange(channel, client, mode, adding, targetNick);
		} break;
	}
}

void Server::handleSimpleMode(Channel* channel, Client* client, char mode, bool adding)
{
	switch (mode) {
		case 'i': // Invite-only
			channel->setInviteOnly(adding);
			break;
		case 't': // Topic protection
			channel->setTopicRestriction(adding);
			break;
	}
	broadcastModeChange(channel, client, mode, adding);
}

void Server::handleChannelMode(Client* client, Channel* channel,
							   const std::string& channelName,
							   const std::string& modeString,
							   std::vector< std::string >& modeParams)
{
	// Check if the client is a channel operator
	if (!channel->getOperator(client->getFd())) {
		sendError(482, client->getNickname(), channelName, client->getFd(), " :You're not channel operator\r\n");
		return;
	}

	bool adding = true; // True for '+', false for '-'
	size_t paramIndex = 0;

	for (size_t i = 0; i < modeString.length(); ++i) {
		char c = modeString[i];

		if (c == '+') {
			adding = true;
			continue;
		} else if (c == '-') {
			adding = false;
			continue;
		}

		switch (c) {
			case 'i':
			case 't':
				handleSimpleMode(channel, client, c, adding);
				break;
			case 'k':
			case 'l':
			case 'o':
				handleParamMode(channel, client, c, adding, modeParams, paramIndex);
				break;
			default:
				sendResponse(ERR_UNKNOWNMODE(std::string(1,c), channelName), client->getFd());
				break;
		}
	}
}

void Server::modeCommand(std::string& cmd, int fd)
{
	Client* client = getClientFromFd(fd);
	if (!client) {
		return;
	}

	// Trim leading and trailing whitespace
	std::string cmd_line = trim(cmd);

	// Remove "MODE" from the command
	const std::string mode_cmd = "MODE";
	size_t pos = cmd_line.find(mode_cmd);
	if (pos != 0) {
		return;
	}

	pos += mode_cmd.length();

	// Skip whitespace
	while (pos < cmd_line.length() && isspace(cmd_line[pos])) {
		pos++;
	}

	if (pos >= cmd_line.length()) {
		// No parameters provided
		sendError(461, client->getNickname(), client->getFd(), " :Not enough parameters\r\t");
		return;
	}

	// Extract the rest of the command
	std::string params = cmd_line.substr(pos);

	// Split the parameters into tokens
	std::vector< std::string > tokens = split(params, ' ');


	std::string channelName = tokens[0];
	Channel* channel = getChannel(channelName);

	if (!channel) {
		sendError(403, client->getNickname(), channelName, client->getFd(), " :No such channel\r\n");
		return;
	}

	if (tokens.size() == 1) {
		// Mode query
		sendResponse(RPL_CHANNELMODES(client->getNickname(),
									  channelName,
									  channel->getChannelSettedModes()) +
							 RPL_CREATIONTIME(client->getNickname(),
											  channelName,
											  channel->getChannelCreationTime()),
					 fd);
		return;
	}

	if (tokens.size() < 2) {
		// Need at least a channel name and mode string
		sendError(461, client->getNickname(), client->getFd(), " :Not enough parameters\r\t");
		return;
	}

	std::string modeString = tokens[1];

	// Collect any additional parameters
	std::vector< std::string > modeParams;
	for (size_t i = 2; i < tokens.size(); ++i) {
		modeParams.push_back(tokens[i]);
	}

	// Proceed to handle the mode changes
	handleChannelMode(client, channel, channelName, modeString, modeParams);
}
