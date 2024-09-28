#include "Client.hpp"

Client::Client() : mFd(-1),
				   mIdleTime(0.0),
				   mLastActiveTime(0),
				   mIsOperator(false),
				   mIsAuthenticated(false),
				   mIsLoggedIn(false)
{
	mNickname = "";
	mUsername = "";
	mIpAddr = "";
	mCmdBuffer = "";
}

Client::Client(const Client& other)
{
	if (this != &other) {
		mFd = other.mFd;
		mIdleTime = other.mIdleTime;
		mLastActiveTime = other.mLastActiveTime;
		mIpAddr = other.mIpAddr;
		mIsOperator = other.mIsOperator;
		mIsAuthenticated = other.mIsAuthenticated;
		mIsLoggedIn = other.mIsLoggedIn;
		mNickname = other.mNickname;
		mUsername = other.mUsername;
		mCmdBuffer = other.mCmdBuffer;
		mChannelInvites = other.mChannelInvites;
	}
	return;
}

Client::~Client() {}

Client& Client::operator=(const Client& rhs)
{
	// Guard Self-Assignment
	if (this == &rhs) {
		return *this;
	}
	mFd = rhs.mFd;
	mIdleTime = rhs.mIdleTime;
	mLastActiveTime = rhs.mLastActiveTime;
	mIpAddr = rhs.mIpAddr;
	mIsOperator = rhs.mIsOperator;
	mIsAuthenticated = rhs.mIsAuthenticated;
	mIsLoggedIn = rhs.mIsLoggedIn;
	mNickname = rhs.mNickname;
	mUsername = rhs.mUsername;
	mCmdBuffer = rhs.mCmdBuffer;
	mChannelInvites = rhs.mChannelInvites;
	return *this;
}

int Client::getFd() const
{
	return mFd;
}

double Client::getIdleTime() const
{
	return mIdleTime;
}

time_t Client::getLastActiveTime() const
{
	return mLastActiveTime;
}

bool Client::getIsInvitedToChannel(std::string channel_name) const
{
	for (size_t i = 0; i < mChannelInvites.size(); ++i) {
		if (mChannelInvites[i] == channel_name) {
			return true;
		}
	}
	return false;
}

bool Client::getIsOperator() const
{
	return mIsOperator;
}

bool Client::getIsAuthenticated() const
{
	return mIsAuthenticated;
}

bool Client::getIsLoggedIn() const
{
	return mIsLoggedIn;
}

std::string Client::getNickname() const
{
	return mNickname;
}

std::string Client::getUsername() const
{
	return mUsername;
}

std::string Client::getIpAddr() const
{
	return mIpAddr;
}

std::string Client::getFullMask() const
{
	return mNickname + "!~" + mUsername + "@localhost";
}


std::string Client::getCmdBuffer() const
{
	return mCmdBuffer;
}

void Client::clearCmdBuffer()
{
	mCmdBuffer.clear();
}

void Client::setCmdBuffer(std::string cmd_received)
{
	mCmdBuffer += cmd_received;
}

void Client::setIsOperator(bool is_operator)
{
	mIsOperator = is_operator;
}

void Client::setIsAuthenticated(bool is_authenticated)
{
	mIsAuthenticated = is_authenticated;
}

void Client::setIsLoggedIn(bool is_logged_in)
{
	mIsLoggedIn = is_logged_in;
}

void Client::setFd(int fd)
{
	mFd = fd;
}

void Client::setIdleTime(double idle)
{
	mIdleTime = idle;
}

void Client::setLastActiveTime(time_t time_)
{
	mLastActiveTime = time_;
}

void Client::setIpAddr(std::string ipaddr)
{
	mIpAddr = ipaddr;
}

void Client::setNickname(std::string nickname)
{
	mNickname = nickname;
}

void Client::setUsername(std::string username)
{
	mUsername = username;
}

void Client::addChannelInvite(std::string channel_name)
{
	mChannelInvites.push_back(channel_name);
}

void Client::removeChannelInvite(std::string channel_name)
{
	for (size_t i = 0; i < mChannelInvites.size(); ++i) {
		if (mChannelInvites[i] == channel_name) {
			mChannelInvites.erase(mChannelInvites.begin() + i);
			break;
		}
	}
}
