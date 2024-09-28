#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <ctime>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <vector>

class Client;

class Channel {
public:
	Channel();                     // Canonical Default Constructor
	Channel(const Channel& other); // Canonical Copy Constructor
	~Channel();                    // Canonical Default Destructor

	Channel& operator=(const Channel& rhs); // Canonical Assignment Operator

	// Setters
	void setInviteOnly(bool is_invite_only);
	void setChannelProtected(bool is_protected);
	void setTopicRestriction(bool is_topic_restricted);
	void setChannelLimited(bool is_limited);
	void setLimit(int limit);
	void setTopicName(std::string topic_name);
	void setKey(std::string channel_key);
	void setChannelName(std::string channel_name);
	void setChannelTime(std::string channel_time);
	void setChannelMode(char mode, bool value);
	void setCreationTime();

	// Getters
	int getClientsNumber() const;
	int getChannelLimit() const;
	bool getInviteOnly() const;
	bool getTopicRestriction() const;
	bool getChannelProtected() const;
	bool getChannelLimitted() const;
	bool getClientInChannel(std::string nickname) const;
	std::string getTopicName() const;
	std::string getTime() const;
	std::string getChannelKey() const;
	std::string getChannelName() const;
	std::string getChannelCreationTime() const;
	std::string getChannelSettedModes() const;
	std::string getChannelClientList() const;
	std::vector< int > getClientFds() const;

	Client* getClient(int fd);
	Client* getOperator(int fd);
	Client* returnClientInChannel(std::string nickname);

	// Member Functions
	void addClient(const Client& newClient);
	void addOperator(const Client& newOperator);
	void removeClient(int fd);
	void removeOperator(int fd);
	bool changeClientToOperator(std::string& nickname);
	bool changeOperatorToClient(std::string& nickname);

	// Send messages to all
	void broadcastMessage(std::string message);
	void broadcastMessage(std::string message, int fd);

private:
	int mChannelLimit;
	std::string mChannelName;
	std::string mTime;
	std::string mChannelKey;
	std::string mCreatedAt;
	std::string mTopicName;
	std::vector< Client > mClientList;
	std::vector< Client > mOperatorList;
	std::map< char, bool > mModes;
};

#endif // CHANNEL_HPP
