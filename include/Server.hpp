#ifndef SERVER_HPP
#define SERVER_HPP

#include "Channel.hpp"
#include "Client.hpp"
#include "ServerResponses.hpp"
#include <arpa/inet.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <set>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#define RED "\e[1;31m"
#define GREEN "\e[1;32m"
#define YELLOW "\e[1;33m"
#define WHITE "\033[0m"

class Server {
public:
	Server();                    // Canonical Default Constructor
	Server(const Server& other); // Canonical Copy Constructor
	~Server();                   // Canonical Default Destructor

	Server& operator=(const Server& rhs); // Canonical Assignment Operator

	void serverInit(int port, std::string password);
	void createServerSocket();
	void acceptNewClient();
	void receiveNewData(int fd);

	static void signalHandler(int signal);

	bool isClientRegister(int fd);

	Client* getClientFromFd(int fd);
	Client* getClientFromNickname(std::string nickname);
	Channel* getChannel(std::string channel_name);

	std::vector< std::string > splitCmdBuffer(std::string cmd_buffer);
	std::vector< std::string > tokenizeCommand(std::string cmd);
	void tokenizeAndExecCmd(std::string cmd_chain, int fd);

	// Execute commands
	void authenticateClient(std::string& cmd, int fd);
	void setNickname(std::string& cmd, int fd);
	void setUsername(std::string& cmd, int fd);
	void quitCommand(std::string& cmd, int fd);
	void kickCommand(std::string& cmd, int fd);
	void joinCommand(std::string& cmd, int fd);
	void topicCommand(std::string& cmd, int fd);
	void modeCommand(std::string& cmd, int fd);
	void partCommand(std::string& cmd, int fd);
	void privmsgCommand(std::string& cmd, int fd);
	void inviteCommand(std::string& cmd, int fd);
	void pingCommand(std::string& cmd, int fd);
	void pongCommand(std::string& cmd, int fd);
	void whoisCommand(std::string& cmd, int fd);
	void joinChannel(Client* client, std::string& channelName, std::string& key);
	void partAllChannels(int fd);
	void sendNamesList(Client* client, Channel* channel);
	void handleChannelMode(Client* client, Channel* channel,
						   const std::string& channelName,
						   const std::string& modeString,
						   std::vector< std::string >& modeParams);
	void handleSimpleMode(Channel* channel, Client* client,
						  char mode, bool adding);
	void broadcastModeChange(Channel* channel, Client* client,
							 char mode, bool adding, const std::string& param = "");
	void handleParamMode(Channel* channel, Client* client,
						 char mode, bool adding,
						 std::vector< std::string >& modeParams,
						 size_t& paramIndex);
	void partChannel(Client* client, const std::string& channelName,
					 const std::string& reason);
	void handleInviteCommand(Client* inviter, const std::string& targetNick,
							 const std::string& channelName);
	void handleClientActivity(Client* client);
	void updateIdleTime(Client* client);

	// Helper Function
	bool trailingParameter(std::string& cmd);
	bool isValidNickname(std::string& nick);
	bool isNicknameInUse(std::string& nick);
	bool isClientValidAuthentication(Client* client);

	// Errors
	void sendError(int code, std::string clientname, int fd, std::string msg);
	void sendError(int code, std::string clientname, std::string channelname,
				   int fd, std::string msg);

	// Server Responses
	void sendResponse(std::string response, int fd);

	// Getters
	int getPort() const;
	std::string getPassword() const;
	int getServerSocketFd() const;

	// Setters
	void setPort(int port);
	void setPassword(std::string password);
	void setServerSocketFd(int serverSocketFd);

	// Member Functions
	void addPollFd(struct pollfd newFd);
	void removeFd(int fd);
	void addClient(Client newClient);
	void removeClient(int fd);
	void addChannel(Channel newChannel);
	void removeChannel(std::string channel_name);
	void removeClientFromChannels(int fd, std::string reason);
	void closeClientsFds();
	void cleanUpChannels();
	void closeConnection(int fd);

private:
	int mPort;
	std::string mServerPassword;
	int mServerSocketFd;
	static bool mSignaled;
	std::vector< Client > mClients;
	std::vector< Channel > mChannels;
	std::vector< struct pollfd > mFds;
	struct sockaddr_in addr;
	struct sockaddr_in clientAddr;
	struct pollfd newClient;
};

typedef void (Server::*ExecCMD)(std::string&, int);

#define CALL_EXEC_CMD_FUNC(object, ptrToMember) ((object).*(ptrToMember))

#endif // SERVER_HPP
