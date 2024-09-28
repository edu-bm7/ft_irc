#include "Server.hpp"
#include "ServerResponses.hpp"

bool Server::trailingParameter(std::string& cmd)
{
	if (cmd[0] == ':') {
		return true;
	}
	return false;
}

bool Server::isClientValidAuthentication(Client* client)
{
	if (client && client->getIsAuthenticated() && !client->getUsername().empty() && !client->getNickname().empty() && client->getNickname() != "*" && !client->getIsLoggedIn()) {
		return true;
	}
	return false;
}

void Server::closeConnection(int fd)
{
	// Notify the client about the disconnection (optional)
	std::string quitMsg = ":localhost QUIT :Authentication failed\r\n";
	if (send(fd, quitMsg.c_str(), quitMsg.size(), 0) == -1) {
		std::cerr << "Response send() failed" << std::endl;
	}

	// Remove the client from the clients list
	removeClient(fd);
	removeFd(fd);

	// Close the socket
	close(fd);
	fd = -1;
}

void Server::authenticateClient(std::string& cmd, int fd)
{
	Client* client = getClientFromFd(fd);
	cmd = cmd.substr(std::strlen("PASS"));
	size_t pos = cmd.find_first_not_of(" \t\v");
	if (pos < cmd.size()) {
		cmd = cmd.substr(pos);
		if (trailingParameter(cmd)) {
			cmd.erase(cmd.begin());
		}
	}
	if (pos == std::string::npos || cmd.empty()) {
		sendResponse(ERR_NEEDMOREPARAMS(std::string("*"), cmd), fd);
		closeConnection(fd);
	} else if (!client->getIsAuthenticated()) {
		if (cmd == getPassword()) {
			client->setIsAuthenticated(true);
		} else {
			sendResponse(ERR_INCORPASS(std::string("*")), fd);
			closeConnection(fd);
		}
	} else {
		sendResponse(ERR_ALREADYREGISTERED(client->getNickname()), fd);
	}
}

bool Server::isValidNickname(std::string& nick)
{
	// Allowed special characters
	const std::string validSpecialChars = "-[]\\`^_{}";

	for (size_t i = 1; i < nick.length(); ++i) {
		char c = nick[i];
		if (!std::isalnum(c) && validSpecialChars.find(c) == std::string::npos) {
			return false;
		}
	}
	return true;
}

bool Server::isNicknameInUse(std::string& nick)
{
	for (size_t i = 0; i < mClients.size(); ++i) {
		if (mClients[i].getNickname() == nick) {
			return true;
		}
	}
	return false;
}

void Server::setNickname(std::string& cmd, int fd)
{
	Client* client = getClientFromFd(fd);
	cmd = cmd.substr(std::strlen("NICK"));
	size_t pos = cmd.find_first_not_of(" \t\v");
	if (pos < cmd.size()) {
		cmd = cmd.substr(pos);
		if (trailingParameter(cmd)) {
			cmd.erase(cmd.begin());
		}
	}
	if (pos == std::string::npos || cmd.empty()) {
		sendResponse(ERR_NEEDMOREPARAMS(std::string("*"), cmd), fd);
		return;
	}
	if (!std::isalpha(cmd[0]) || cmd.length() > 9) {
		sendResponse(ERR_ERRONEUSNICK(std::string(cmd)), fd);
		return;
	}
	if (!isValidNickname(cmd)) {
		sendResponse(ERR_ERRONEUSNICK(std::string(cmd)), fd);
		return;
	}
	if (isNicknameInUse(cmd) && client->getNickname() != cmd) {
		std::string inUse = "*";
		if (client->getNickname().empty()) {
			client->setNickname(inUse);
		}
		sendResponse(ERR_NICKINUSE(client->getNickname(), cmd), fd);
		return;
	} else {
		if (client && client->getIsAuthenticated()) {
			std::string oldNick = client->getNickname();
			client->setNickname(cmd);
			if (!oldNick.empty() && oldNick != cmd) {
				if (oldNick == "*" && !client->getUsername().empty()) {
					client->setIsLoggedIn(true);
					sendResponse(RPL_CONNECTED(client->getNickname()), fd);
					sendResponse(RPL_NICKCHANGE(oldNick, client->getNickname()), fd);
				} else {
					sendResponse(RPL_NICKCHANGE(oldNick, cmd), fd);
				}
			}
		} else if (client && !client->getIsAuthenticated()) {
			sendResponse(ERR_NOTREGISTERED(cmd), fd);
		}
	}
	if (isClientValidAuthentication(client)) {
		client->setIsLoggedIn(true);
		sendResponse(RPL_CONNECTED(client->getNickname()), fd);
	}
}

void Server::setUsername(std::string& cmd, int fd)
{
	std::vector< std::string > tokenizedCmd = tokenizeCommand(cmd);
	Client* client = getClientFromFd(fd);
	if (client && tokenizedCmd.size() < 5) {
		sendResponse(ERR_NEEDMOREPARAMS(client->getNickname(), cmd), fd);
		return;
	}
	if (!client || !client->getIsAuthenticated()) {
		sendResponse(ERR_NOTREGISTERED(std::string("*")), fd);
	} else if (client && !client->getUsername().empty()) {
		sendResponse(ERR_ALREADYREGISTERED(client->getNickname()), fd);
		return;
	} else {
		client->setUsername(tokenizedCmd[1]);
	}
	if (isClientValidAuthentication(client)) {
		client->setIsLoggedIn(true);
		sendResponse(RPL_CONNECTED(client->getNickname()), fd);
	}
}
