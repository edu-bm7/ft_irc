#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel() : mChannelLimit(0),
					 mChannelName(""),
					 mTime(""),
					 mChannelKey(""),
					 mCreatedAt(""),
					 mTopicName("")
{
	mModes['i'] = false;
	mModes['k'] = false;
	mModes['l'] = false;
	mModes['o'] = false;
	mModes['t'] = false;
	return;
}

Channel::~Channel() {}

Channel::Channel(const Channel& other) : mChannelLimit(other.mChannelLimit),
										 mChannelName(other.mChannelName),
										 mTime(other.mTime),
										 mChannelKey(other.mChannelKey),
										 mCreatedAt(other.mCreatedAt),
										 mTopicName(other.mTopicName),
										 mClientList(other.mClientList),
										 mOperatorList(other.mOperatorList),
										 mModes(other.mModes)
{
	return;
}

Channel& Channel::operator=(const Channel& rhs)
{
	// Guard Self-Assignment
	if (this == &rhs) {
		return *this;
	}

	mChannelLimit = rhs.mChannelLimit;
	mChannelName = rhs.mChannelName;
	mTime = rhs.mTime;
	mChannelKey = rhs.mChannelKey;
	mCreatedAt = rhs.mCreatedAt;
	mTopicName = rhs.mTopicName;
	mClientList = rhs.mClientList;
	mOperatorList = rhs.mOperatorList;
	mModes = rhs.mModes;

	return *this;
}

// Setters
void Channel::setInviteOnly(bool is_invite_only)
{
	mModes['i'] = is_invite_only;
}

void Channel::setChannelProtected(bool is_protected)
{
	mModes['k'] = is_protected;
}

void Channel::setChannelLimited(bool is_limited)
{
	mModes['l'] = is_limited;
}

void Channel::setTopicRestriction(bool is_topic_restricted)
{
	mModes['t'] = is_topic_restricted;
}

void Channel::setTopicName(std::string topic_name)
{
	mTopicName = topic_name;
}

void Channel::setKey(std::string channel_key)
{
	mChannelKey = channel_key;
}

void Channel::setLimit(int limit)
{
	mChannelLimit = limit;
}

void Channel::setChannelName(std::string channel_name)
{
	mChannelName = channel_name;
}

void Channel::setChannelTime(std::string channel_time)
{
	mTime = channel_time;
}

void Channel::setChannelMode(char mode, bool value)
{
	std::map< char, bool >::iterator it = mModes.find(mode);
	if (it != mModes.end()) {
		it->second = value;
	} else {
		std::cerr << "This Mode is invalid: " << mode << std::endl;
	}
}

void Channel::setCreationTime()
{
	std::time_t mTime = std::time(NULL);
	std::ostringstream oss;

	oss << mTime;
	mCreatedAt = std::string(oss.str());
}

bool Channel::getInviteOnly() const
{
	std::map< char, bool >::const_iterator it = mModes.find('i');
	if (it != mModes.end()) {
		return it->second;
	} else {
		throw(std::runtime_error("This Mode is invalid"));
	}
}

bool Channel::getTopicRestriction() const
{
	std::map< char, bool >::const_iterator it = mModes.find('t');
	if (it != mModes.end()) {
		return it->second;
	} else {
		throw(std::runtime_error("This Mode is invalid"));
	}
}

bool Channel::getChannelProtected() const
{
	std::map< char, bool >::const_iterator it = mModes.find('k');
	if (it != mModes.end()) {
		return it->second;
	} else {
		throw(std::runtime_error("This Mode is invalid"));
	}
}

bool Channel::getChannelLimitted() const
{
	std::map< char, bool >::const_iterator it = mModes.find('l');
	if (it != mModes.end()) {
		return it->second;
	} else {
		throw(std::runtime_error("This Mode is invalid"));
	}
}

bool Channel::getClientInChannel(std::string nickname) const
{
	for (size_t i = 0; i < mClientList.size(); ++i) {
		if (mClientList[i].getNickname() == nickname) {
			return true;
		}
	}

	for (size_t i = 0; i < mOperatorList.size(); ++i) {
		if (mOperatorList[i].getNickname() == nickname) {
			return true;
		}
	}

	return false;
}

int Channel::getClientsNumber() const
{
	return mClientList.size() + mOperatorList.size();
}

int Channel::getChannelLimit() const
{
	return mChannelLimit;
}

std::string Channel::getTopicName() const
{
	return mTopicName;
}

std::string Channel::getTime() const
{
	return mTime;
}

std::string Channel::getChannelKey() const
{
	return mChannelKey;
}

std::string Channel::getChannelName() const
{
	return mChannelName;
}

std::string Channel::getChannelCreationTime() const
{
	return mCreatedAt;
}

std::string Channel::getChannelSettedModes() const
{
	std::string modes;
	std::map< char, bool >::const_iterator it;
	for (it = mModes.begin(); it != mModes.end(); ++it) {
		if (it->first != 'o' && it->second == true) {
			modes.push_back(it->first);
		}
	}
	if (!modes.empty()) {
		modes.insert(modes.begin(), '+');
	}
	return modes;
}

std::string Channel::getChannelClientList() const
{
	std::string list;
	if (!mOperatorList.empty()) {
		for (size_t i = 0; i < mOperatorList.size(); ++i) {
			list += "@" + mOperatorList[i].getNickname();
			if (i + 1 < mOperatorList.size()) {
				list += " ";
			}
		}
	}


	if (!mClientList.empty()) {
		list += " ";
		for (size_t i = 0; i < mClientList.size(); ++i) {
			list += mClientList[i].getNickname();
			if (i + 1 < mClientList.size()) {
				list += " ";
			}
		}
	}

	return list;
}

Client* Channel::getClient(int fd)
{
	std::vector< Client >::iterator it;
	for (it = mClientList.begin(); it != mClientList.end(); ++it) {
		if (it->getFd() == fd) {
			return &(*it);
		}
	}
	return NULL;
}

std::vector< int > Channel::getClientFds() const
{
	std::vector< int > client_fds;

	// Add clients and operators fds of the channel
	for (size_t i = 0; i < mClientList.size(); ++i) {
		client_fds.push_back(mClientList[i].getFd());
	}

	for (size_t i = 0; i < mOperatorList.size(); ++i) {
		client_fds.push_back(mOperatorList[i].getFd());
	}

	return client_fds;
}

Client* Channel::getOperator(int fd)
{
	std::vector< Client >::iterator it;
	for (it = mOperatorList.begin(); it != mOperatorList.end(); ++it) {
		if (it->getFd() == fd) {
			return &(*it);
		}
	}
	return NULL;
}

Client* Channel::returnClientInChannel(std::string nickname)
{
	std::vector< Client >::iterator it;
	for (it = mOperatorList.begin(); it != mOperatorList.end(); ++it) {
		if (it->getNickname() == nickname) {
			return &(*it);
		}
	}

	for (it = mClientList.begin(); it != mClientList.end(); ++it) {
		if (it->getNickname() == nickname) {
			return &(*it);
		}
	}

	return NULL;
}

// Member Functions
void Channel::addClient(const Client& newClient)
{
	mClientList.push_back(newClient);
}

void Channel::addOperator(const Client& newOperator)
{
	mOperatorList.push_back(newOperator);
}

void Channel::removeClient(int fd)
{
	std::vector< Client >::iterator it;
	for (it = mClientList.begin(); it != mClientList.end(); ++it) {
		if (it->getFd() == fd) {
			mClientList.erase(it);
			break;
		}
	}
}

void Channel::removeOperator(int fd)
{
	std::vector< Client >::iterator it;
	for (it = mOperatorList.begin(); it != mOperatorList.end(); ++it) {
		if (it->getFd() == fd) {
			mOperatorList.erase(it);
			break;
		}
	}
}

bool Channel::changeClientToOperator(std::string& nickname)
{
	std::vector< Client >::iterator it;
	for (it = mClientList.begin(); it != mClientList.end(); it++) {
		if (it->getNickname() == nickname) {
			mClientList.erase(it);
			mOperatorList.push_back(*it);
			return true;
		}
	}
	return false;
}

bool Channel::changeOperatorToClient(std::string& nickname)
{
	std::vector< Client >::iterator it;
	for (it = mOperatorList.begin(); it != mOperatorList.end(); it++) {
		if (it->getNickname() == nickname) {
			mOperatorList.erase(it);
			mClientList.push_back(*it);
			return true;
		}
	}
	return false;
}

// Send messages to all
void Channel::broadcastMessage(std::string message)
{
	for (size_t i = 0; i < mOperatorList.size(); ++i) {
		if (send(mOperatorList[i].getFd(), message.c_str(), message.size(), 0) == -1) {
			std::cerr << "send() Failed" << std::endl;
		}
	}

	for (size_t i = 0; i < mClientList.size(); i++) {
		if (send(mClientList[i].getFd(), message.c_str(), message.size(), 0) == -1) {
			std::cerr << "send() Failed" << std::endl;
		}
	}
}

// Send messages to all except the user
void Channel::broadcastMessage(std::string message, int fd)
{
	for (size_t i = 0; i < mOperatorList.size(); ++i) {
		if (mOperatorList[i].getFd() != fd) {
			if (send(mOperatorList[i].getFd(), message.c_str(), message.size(), 0) == -1) {
				std::cerr << "send() Failed" << std::endl;
			}
		}
	}

	for (size_t i = 0; i < mClientList.size(); i++) {
		if (mClientList[i].getFd() != fd) {
			if (send(mClientList[i].getFd(), message.c_str(), message.size(), 0) == -1) {
				std::cerr << "send() Failed" << std::endl;
			}
		}
	}
}
