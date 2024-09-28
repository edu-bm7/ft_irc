#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Client {
public:
	Client();                    // Canonical Default Constructor
	Client(const Client& other); // Canonical Copy Constructor
	~Client();                   // Canonical Default Destructor

	Client& operator=(const Client& rhs); // Canonical Assignment Operator

	void addChannelInvite(std::string channel_name);
	void removeChannelInvite(std::string channel_name);

	// Getters
	int getFd() const;
	double getIdleTime() const;
	time_t getLastActiveTime() const;
	bool getIsOperator() const;
	bool getIsAuthenticated() const;
	bool getIsLoggedIn() const;
	bool getIsInvitedToChannel(std::string channel_name) const;
	std::string getNickname() const;
	std::string getUsername() const;
	std::string getIpAddr() const;
	std::string getFullMask() const;
	std::string getCmdBuffer() const;

	// Setters
	void setFd(int fd);
	void setIpAddr(std::string ipaddr);
	void setIsOperator(bool is_operator);
	void setIsAuthenticated(bool is_authenticated);
	void setIsLoggedIn(bool is_logged_in);
	void setCmdBuffer(std::string cmd_received);
	void clearCmdBuffer();
	void setNickname(std::string nickname);
	void setUsername(std::string username);
	void setIdleTime(double idle);
	void setLastActiveTime(time_t time);

private:
	int mFd;
	double mIdleTime;
	time_t mLastActiveTime;
	bool mIsOperator;
	bool mIsAuthenticated;
	bool mIsLoggedIn;
	std::string mNickname;
	std::string mUsername;
	std::string mIpAddr;
	std::string mCmdBuffer;
	std::vector< std::string > mChannelInvites;
};

#endif // CLIENT_HPP
