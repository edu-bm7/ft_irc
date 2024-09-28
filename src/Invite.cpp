#include "Server.hpp"
#include "helper_functions.hpp"

void Server::handleInviteCommand(Client* inviter, const std::string& targetNick,
								 const std::string& channelName)
{
	// Check if the target user exists
	Client* targetClient = getClientFromNickname(targetNick);
	if (!targetClient) {
		sendError(401, inviter->getNickname(), targetNick, inviter->getFd(), " :No such nick/channel\r\n");
		return;
	}

	// Validate the channel name
	if (!isValidChannelName(channelName)) {
		sendError(403, inviter->getNickname(), channelName, inviter->getFd(), " :No such channel\r\n");
		return;
	}

	// Check if the channel exists
	Channel* channel = getChannel(channelName);
	if (channel) {
		// Channel exists; perform additional checks

		// Check if the inviter is a member of the channel
		if (!channel->getClientInChannel(inviter->getNickname())) {
			sendError(442, inviter->getNickname(), channelName, inviter->getFd(), " :You're not on that channel\n\r");
			return;
		}

		// If channel is invite-only, ensure inviter is an operator
		if (channel->getInviteOnly() && !channel->getOperator(inviter->getFd())) {
			sendError(482, inviter->getNickname(), channelName, inviter->getFd(), " :You're not channel operator\r\n");
			return;
		}
	}

	// Check if the target user is already in the channel
	if (channel && channel->getClientInChannel(targetClient->getNickname())) {
		sendError(443, inviter->getNickname(), targetNick + " " + channelName, inviter->getFd(), " :is already on channel\r\n");
		return;
	}

	// Check if the user has already been invited
	if (targetClient->getIsInvitedToChannel(channelName)) {
		sendError(443, inviter->getNickname(), targetNick + " " + channelName, inviter->getFd(), " :is already invited to channel\r\n");
		return;
	}

	// Add the invite
	targetClient->addChannelInvite(channelName);

	// Notify the target user
	std::string inviteMsg = ":" + inviter->getFullMask() + " INVITE " + targetNick + " " + channelName + "\r\n";
	sendResponse(inviteMsg, targetClient->getFd());

	// Notify the inviter that the invite was successful
	std::stringstream ss;
	ss << ": 341 " << inviter->getNickname() << " " << targetNick << " " << channelName << " :Invitation sent\r\n";
	std::string inviteSuccessMsg = ss.str();
	sendResponse(inviteSuccessMsg, inviter->getFd());
}

void Server::inviteCommand(std::string& cmd, int fd)
{
	Client* inviter = getClientFromFd(fd);
	if (!inviter) {
		return;
	}

	// Trim leading and trailing whitespace
	std::string cmd_line = trim(cmd);

	// Remove "INVITE" from the command
	const std::string invite_cmd = "INVITE";
	size_t pos = cmd.find(invite_cmd);
	if (pos != 0) {
		return;
	}

	pos += invite_cmd.length();

	// Skip whitespace
	while (pos < cmd.length() && isspace(cmd[pos])) {
		pos++;
	}

	if (pos >= cmd.length()) {
		sendError(461, inviter->getNickname(), inviter->getFd(), " :Not enough parameters\r\t");
		return;
	}

	// Extract the parameters
	std::string params = cmd.substr(pos);
	params = trim(params);

	// Split parameters into nickname and channel
	std::vector< std::string > tokens = split(params, ' ');

	if (tokens.size() < 2) {
		// Need both nickname and channel
		sendError(461, inviter->getNickname(), inviter->getFd(), " :Not enough parameters\r\t");
		return;
	}

	std::string targetNick = tokens[0];
	std::string channelName = tokens[1];

	// Proceed to handle the invite
	handleInviteCommand(inviter, targetNick, channelName);
}
