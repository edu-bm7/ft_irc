#include "Server.hpp"
#include <stdexcept>

Server::Server()
{
	mServerSocketFd = -1;
}

Server::~Server()
{
	try {
		// Close Clients connections
		closeClientsFds();

		// Clean up Channels
		cleanUpChannels();

		// Close the server socket
		if (mServerSocketFd != -1) {
			std::cout << RED << "Server <"
					  << mServerSocketFd
					  << "> Disconnected" << WHITE << std::endl;
			close(mServerSocketFd);
			setServerSocketFd(-1);
		}
	} catch (...) {
		// Suppress all exceptions to prevent termination during stack unwinding
	}
}

Server::Server(const Server& other)
{
	*this = other;
	return;
}

Server& Server::operator=(const Server& rhs)
{
	// Guard Self-Assignment
	if (this == &rhs) {
		return *this;
	}
	mPort = rhs.mPort;
	mServerSocketFd = rhs.mServerSocketFd;
	mServerPassword = rhs.mServerPassword;
	mClients = rhs.mClients;

	return *this;
}

void Server::sendResponse(std::string response, int fd)
{
	std::cout << "Response:\n"
			  << response;
	if (send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() failed" << std::endl;
}

void Server::sendError(int code, std::string clientname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << msg;
	std::string resp = ss.str();
	if (send(fd, resp.c_str(), resp.size(), 0) == -1) {
		std::cerr << "send() Failed" << std::endl;
	}
}

void Server::sendError(int code, std::string clientname, std::string channelname,
					   int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << " " << channelname << msg;
	std::string resp = ss.str();
	if (send(fd, resp.c_str(), resp.size(), 0) == -1) {
		std::cerr << "send() Failed" << std::endl;
	}
}

int Server::getPort() const
{
	return mPort;
}

std::string Server::getPassword() const
{
	return mServerPassword;
}

int Server::getServerSocketFd() const
{
	return mServerSocketFd;
}

void Server::setPort(int port)
{
	mPort = port;
}

void Server::setPassword(std::string password)
{
	mServerPassword = password;
}

void Server::setServerSocketFd(int serverSocketFd)
{
	mServerSocketFd = serverSocketFd;
}

void Server::addPollFd(struct pollfd newFd)
{
	mFds.push_back(newFd);
}

void Server::addClient(Client newClient)
{
	mClients.push_back(newClient);
}

void Server::removeClient(int fd)
{
	// remove the client from the mClients vector
	for (size_t i = 0; i < mClients.size(); ++i) {
		if (mClients[i].getFd() == fd) {
			mClients.erase(mClients.begin() + i);
			break;
		}
	}
}

void Server::removeFd(int fd)
{
	for (size_t i = 0; i < mFds.size(); ++i) {
		if (mFds[i].fd == fd) {
			mFds.erase(mFds.begin() + i);
			break;
		}
	}
}

Client* Server::getClientFromFd(int fd)
{
	for (size_t i = 0; i < mClients.size(); ++i) {
		if (mClients[i].getFd() == fd) {
			return &mClients[i];
		}
	}
	return NULL;
}

Client* Server::getClientFromNickname(std::string nickname)
{
	for (size_t i = 0; i < mClients.size(); ++i) {
		if (mClients[i].getNickname() == nickname) {
			return &mClients[i];
		}
	}
	return NULL;
}

Channel* Server::getChannel(std::string channel_name)
{
	for (size_t i = 0; i < mChannels.size(); ++i) {
		if (mChannels[i].getChannelName() == channel_name) {
			return &mChannels[i];
		}
	}
	return NULL;
}

void Server::partAllChannels(int fd)
{
	Client* client = getClientFromFd(fd);
	if (!client) {
		return;
	}
	// Remove the client from all channels and collect clients to notify
	for (size_t i = 0; i < mChannels.size();) {
		Channel& channel = mChannels[i];
		bool clientWasInChannel = false;

		if (channel.getClient(fd)) {
			channel.removeClient(fd);
			clientWasInChannel = true;
		} else if (channel.getOperator(fd)) {
			channel.removeOperator(fd);
			clientWasInChannel = true;
		}


		if (clientWasInChannel) {
			// Notify other channel members
			std::string partMsg = ":" + client->getFullMask() + " PART " + channel.getChannelName() + "\r\n";
			channel.broadcastMessage(partMsg);
			if (channel.getClientsNumber() == 0) {
				mChannels.erase(mChannels.begin() + i);
				continue;
			}
		}
		++i;
	}
}

void Server::removeClientFromChannels(int fd, std::string reason)
{
	// Set to store unique fds of clients to notify
	std::set< int > clientsToNotify;

	// Remove the client from all channels and collect clients to notify
	for (size_t i = 0; i < mChannels.size();) {
		Channel& channel = mChannels[i];
		bool clientWasInChannel = false;

		if (channel.getClient(fd)) {
			channel.removeClient(fd);
			clientWasInChannel = true;
		} else if (channel.getOperator(fd)) {
			channel.removeOperator(fd);
			clientWasInChannel = true;
		}

		if (clientWasInChannel) {
			// Collect fds of other clients in the channel
			std::vector< int > otherClients = channel.getClientFds();
			clientsToNotify.insert(otherClients.begin(), otherClients.end());

			if (channel.getClientsNumber() == 0) {
				mChannels.erase(mChannels.begin() + i);
				continue;
			}
		}
		++i;
	}

	// Construct the QUIT message
	std::string rpl = ":" + getClientFromFd(fd)->getNickname() + "!~" + getClientFromFd(fd)->getUsername() + "@localhost QUIT :" + reason + "\r\n";

	// Send the QUIT message to each client once
	for (std::set< int >::iterator it = clientsToNotify.begin(); it != clientsToNotify.end(); ++it) {
		int client_fd = *it;
		if (send(client_fd, rpl.c_str(), rpl.length(), 0) == -1) {
			std::cerr << "send() Failed" << std::endl;
		}
	}
}

bool Server::mSignaled = false;

void Server::signalHandler(int signal)
{
	(void) signal;
	std::cout << std::endl
			  << "Signal Received" << std::endl;
	Server::mSignaled = true;
}

void Server::closeClientsFds()
{
	// Close all clients
	for (size_t i = 0; i < mClients.size(); ++i) {
		if (mClients[i].getFd() != -1) {

			std::cout << RED << "Client <"
					  << mClients[i].getFd()
					  << "> Disconnected" << WHITE << std::endl;
			close(mClients[i].getFd());
			mClients[i].setFd(-1);
		}
	}
}

void Server::cleanUpChannels()
{
	mChannels.clear();
}

void Server::createServerSocket()
{
	addr.sin_family = AF_INET;         // Set address family to IPv4
	addr.sin_port = htons(mPort);      // Convert the port to network byte order (big endian)
	addr.sin_addr.s_addr = INADDR_ANY; // Set the address to any local machine address

	// Creates the socket and checks if it was successful
	mServerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (mServerSocketFd == -1)
		throw(std::runtime_error("Failed to create socket"));

	// Enables the option REUSEADDR, so the socket can forcebily bind
	// to the same port(usually recentely closed) without waiting for the OS to release it
	int option = 1;
	if (setsockopt(mServerSocketFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) {
		throw(std::runtime_error("Failed to set option (SO_REUSEADDR) on socket"));
	}

	// Set the socket option (O_NONBLOCK) for non-blocking socket
	if (fcntl(mServerSocketFd, F_SETFL, O_NONBLOCK) == -1) {
		throw(std::runtime_error("Failed to set option (O_NONBLOCK) on socket"));
	}

	// Bind the socket to the address
	if (bind(mServerSocketFd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		throw(std::runtime_error("Failed to bind socket"));
	}

	// Listen for connections and make the socket passive
	if (listen(mServerSocketFd, SOMAXCONN) == -1) {
		throw(std::runtime_error("listen() Failed"));
	}

	// Add the server socke to the pollfd
	newClient.fd = mServerSocketFd;
	// Set the event to POLLIN for reading data
	newClient.events = POLLIN;
	// Set the return events initially to 0
	newClient.revents = 0;
	// Push newClient to the vector of pollfd
	addPollFd(newClient);
}

void Server::serverInit(int port, std::string password)
{
	setPort(port);
	setPassword(password);
	createServerSocket(); //-> create the server socket

	std::cout << GREEN << "Server <" << mServerSocketFd << "> Connected" << WHITE << std::endl;
	std::cout << "Waiting to accept a connection...\n";

	while (!Server::mSignaled) {
		// Set poll to wait for any event
		if ((poll(&mFds[0], mFds.size(), -1) == -1 && !Server::mSignaled)) {
			throw(std::runtime_error("poll() failed"));
		}
		// checks for events of POLLIN, if the fd is a ServerSocketFd accept a new Client,
		// else receive new data from a already registered Client
		for (size_t i = 0; i < mFds.size(); ++i) {
			if (mFds[i].revents & POLLIN) {
				if (mFds[i].fd == mServerSocketFd) {
					acceptNewClient();
				} else {
					receiveNewData(mFds[i].fd);
				}
			}
		}
	}
}

void Server::acceptNewClient()
{
	Client client;
	// Initialize to 0
	std::memset(&clientAddr, 0, sizeof(clientAddr));
	socklen_t len = sizeof(clientAddr);

	// accept() extracts the first connection request on the queue of pending connections
	// for the listening socket, mServerSocketFd, creates a new connected socket, and
	// returns a new file descriptor referring to that socket. The newly created
	// socket is not in the listening state. The original socket mServerSocketFd
	// is unaffected by this call.
	int incomingfd = accept(mServerSocketFd, (struct sockaddr*) &clientAddr, &len);
	if (incomingfd == -1) {
		std::cout << RED << "accept() failed" << WHITE << std::endl;
		return;
	}

	// Set the client new Socket to Non blocking
	if (fcntl(incomingfd, F_SETFL, O_NONBLOCK) == -1) {
		std::cout << "fcntl() failed" << std::endl;
		return;
	}

	// Add the new socket to the newClient pollfd
	newClient.fd = incomingfd;
	newClient.events = POLLIN;
	newClient.revents = 0;

	// set the client Fd
	client.setFd(incomingfd);
	// Set the client ip address after converting it to string
	client.setIpAddr(inet_ntoa(clientAddr.sin_addr));
	// Push the new client to the Clients vector
	addClient(client);
	// Push the new fd into the fds vector
	addPollFd(newClient);

	std::cout << GREEN
			  << "Client <"
			  << incomingfd
			  << "> Connected" << WHITE << std::endl;
}

std::vector< std::string > Server::splitCmdBuffer(std::string cmd_buffer)
{
	std::vector< std::string > vec;
	std::istringstream iss(cmd_buffer);
	std::string line;

	while (std::getline(iss, line)) {
		size_t pos = line.find_first_of("\r\n");
		if (pos != std::string::npos) {
			line = line.substr(0, pos);
		}
		vec.push_back(line);
	}
	return vec;
}

void Server::receiveNewData(int fd)
{
	// Byffer fir the received data, we then initialize it to 0
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	Client* client = getClientFromFd(fd);
	std::vector< std::string > cmd_chain;
	// Receive the data
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);

	// Check if the client disconnected
	if (bytes <= 0) {
		std::cout
				<< RED
				<< "Client <"
				<< fd << "> Disconnected" << WHITE << std::endl;
		// Clear the client
		removeClientFromChannels(fd, "");
		removeClient(fd);
		removeFd(fd);
		// Close the client socket
		close(fd);
		fd = -1;
	} else {
		if (client) {
			client->setCmdBuffer(buff);
			// The command isn't finished so we return to continue adding to the buffer
			if (client->getCmdBuffer().find_first_of("\r\n") == std::string::npos) {
				return;
			}
			cmd_chain = splitCmdBuffer(client->getCmdBuffer());
			for (size_t i = 0; i < cmd_chain.size(); ++i) {
				tokenizeAndExecCmd(cmd_chain[i], fd);
			}
			client = getClientFromFd(fd);
			if (client) {
				client->clearCmdBuffer();
			}
		}
	}
}

std::vector< std::string > Server::tokenizeCommand(std::string cmd)
{
	std::vector< std::string > vec;
	std::istringstream iss(cmd);
	std::string token;
	while (iss >> token) {
		vec.push_back(token);
		token.clear();
	}
	return vec;
}

bool Server::isClientRegister(int fd)
{
	if (!getClientFromFd(fd) || getClientFromFd(fd)->getNickname().empty() ||
		getClientFromFd(fd)->getUsername().empty() ||
		getClientFromFd(fd)->getNickname() == "*" ||
		!getClientFromFd(fd)->getIsLoggedIn())
		return false;
	return true;
}

void Server::tokenizeAndExecCmd(std::string cmd_chain, int fd)
{
	if (cmd_chain.empty()) {
		return;
	}
	Client* client = getClientFromFd(fd);
	if (client) {
		handleClientActivity(client);
	}
	std::vector< std::string > tokenizedCmd = tokenizeCommand(cmd_chain);

	size_t find_first_not_white_space = cmd_chain.find_first_not_of(" \t\v");
	if (find_first_not_white_space != std::string::npos) {
		cmd_chain = cmd_chain.substr(find_first_not_white_space);
	}

	ExecCMD unauthorized_cmds[] = {&Server::authenticateClient, &Server::setNickname,
								   &Server::setUsername, &Server::quitCommand};

	std::string unauthorized_cmds_names[] = {"PASS", "NICK", "USER", "QUIT"};

	const size_t unauthorized_len = sizeof(unauthorized_cmds) / sizeof(ExecCMD);

	for (size_t i = 0; i < unauthorized_len; ++i) {
		if (tokenizedCmd[0] == unauthorized_cmds_names[i]) {
			CALL_EXEC_CMD_FUNC(*this, unauthorized_cmds[i])
			(cmd_chain, fd);
			return;
		}
	}

	ExecCMD authorized_cmds[] = {
			&Server::authenticateClient,
			&Server::setNickname,
			&Server::setUsername,
			&Server::quitCommand,
			&Server::kickCommand,
			&Server::joinCommand,
			&Server::topicCommand,
			&Server::modeCommand,
			&Server::partCommand,
			&Server::privmsgCommand,
			&Server::inviteCommand,
			&Server::pingCommand,
			&Server::whoisCommand,
	};
	std::string authorized_cmds_names[] = {
			"PASS",
			"NICK",
			"USER",
			"QUIT",
			"KICK",
			"JOIN",
			"TOPIC",
			"MODE",
			"PART",
			"PRIVMSG",
			"INVITE",
			"PING",
			"WHOIS",
	};
	const size_t authorized_len = sizeof(authorized_cmds) / sizeof(ExecCMD);
	for (size_t i = 0; i < authorized_len; ++i) {
		if (tokenizedCmd[0] == authorized_cmds_names[i]) {
			CALL_EXEC_CMD_FUNC(*this, authorized_cmds[i])
			(cmd_chain, fd);
			return;
		}
	}
	if (isClientRegister(fd)) {
		if (tokenizedCmd.size()) {
			sendResponse(ERR_CMDNOTFOUND(getClientFromFd(fd)->getNickname(), tokenizedCmd[0]), fd);
		}
	} else {
		sendResponse(ERR_NOTREGISTERED(std::string("*")), fd);
	}
}

// Update idle time on each activity
void Server::updateIdleTime(Client* client)
{
	client->setIdleTime(difftime(time(NULL), client->getLastActiveTime()));
}

// Call this function whenever the client sends any command
void Server::handleClientActivity(Client* client)
{
	client->setLastActiveTime(time(NULL));
	updateIdleTime(client);
}
