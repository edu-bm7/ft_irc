#ifndef SERVERRESPONSES_HPP
#define SERVERRESPONSES_HPP

#define CRLF "\r\n"

#define RPL_CONNECTED(nickname) (":localhost 001 " + nickname + " : Welcome to the IRC server!" + CRLF)
#define RPL_CREATIONTIME(nickname, channelname, creationtime) (":localhost 329 " + nickname + " " + channelname + " " + creationtime + CRLF)
#define RPL_CHANNELMODES(nickname, channelname, modes) (":localhost 324 " + nickname + " " + channelname + " " + modes + CRLF)
#define RPL_CHANGEMODE(hostname, channelname, mode, arguments) (":" + hostname + " MODE " + channelname + " " + mode + " " + arguments + CRLF)
#define RPL_NICKCHANGE(oldnickname, nickname) (":" + oldnickname + " NICK " + nickname + CRLF)
#define RPL_JOINMSG(hostname, ipaddress, channelname) (":" + hostname + "@" + ipaddress + " JOIN " + channelname + CRLF)
#define RPL_NAMREPLY(nickname, channelname, clientslist) (":localhost 353 " + nickname + " @ " + channelname + " :" + clientslist + CRLF)
#define RPL_ENDOFNAMES(nickname, channelname) (":localhost 366 " + nickname + " " + channelname + " :END of /NAMES list" + CRLF)
#define RPL_TOPICIS(nickname, channelname, topic) (":localhost 332 " + nickname + " " + channelname + " :" + topic + "\r\n")
#define RPL_PONG(server) ("PONG :" + server + CRLF)
#define RPL_PING(server) ("PING :" + server + CRLF)
#define RPL_WHOISUSER(nickname, targetnick, targetuser) (":localhost 311 " + nickname + " " + targetnick + " " + targetuser + " localhost * :*" + CRLF)
#define RPL_WHOISSERVER(nickname, targetnick, targetuser) (":localhost 312 " + nickname + " " + targetnick + " " + " localhost :This server is operated by localhost" + CRLF)
#define RPL_WHOISOPERATOR(nickname, targetnick) (":localhost 313 " + nickname + " " + targetnick + " :is an IRC operator" + CRLF)

#define RPL_WHOISIDLE(nickname, targetnick, idletime) (":localhost 317 " + nickname + " " + targetnick + " " + idletime + " :seconds idle" + CRLF)
#define RPL_WHOISCHANNELS(nickname, targetnick, channels) (":localhost 319 " + nickname + " " + targetnick + " :" + channels + CRLF)
#define RPL_ENDOFWHOIS(nickname, targetnick) (":localhost 318 " + nickname + " " + targetnick + " :End of WHOIS list" + CRLF)

/* ------------- ERRORS ----------------- */
#define ERR_NEEDMODEPARM(channelname, mode) (":localhost 696 " + channelname + " * You must specify a parameter for the key mode. " + mode + CRLF)
#define ERR_INVALIDMODEPARM(channelname, mode) (":localhost 696 " + channelname + " Invalid mode parameter. " + mode + CRLF)
#define ERR_KEYSET(nickname, channelname) (":localhost 467 " + nickname + " " + channelname + " :Channel key already set." + CRLF)
#define ERR_NEEDMOREPARAMS(nickname, command) (":localhost 461 " + nickname + " " + command + " :Not enough parameters." + CRLF)
#define ERR_CHANNELNOTFOUND(nickname, channelname) (":localhost 403 " + nickname + " " + channelname + " :No such channel" + CRLF)
#define ERR_NOTOPERATOR(nickname, channelname) (":localhost 482 " + nickname + " " + channelname + " :You're not a channel operator" + CRLF)
#define ERR_NOSUCHNICK(nickname, targetnick) (":localhost 401 " + nickname + " " + targetnick + " :No such nick/channel" + CRLF)
#define ERR_INCORPASS(nickname) (":localhost 464 " + nickname + " :Password incorrect!" + CRLF)
#define ERR_ALREADYREGISTERED(nickname) (":localhost 462 " + nickname + " :You may not reregister!" + CRLF)
#define ERR_NONICKNAME(nickname) (":localhost 431 " + nickname + " :No nickname given" + CRLF)
#define ERR_NICKINUSE(nickname, targetnick) (":localhost 433 " + nickname + " " + targetnick + " :Nickname is already in use" + CRLF)
#define ERR_ERRONEUSNICK(nickname) (":localhost 432 " + nickname + " :Erroneous nicknaeme" + CRLF)
#define ERR_NOTREGISTERED(nickname) (":localhost 451 " + nickname + " :You have not registered!" + CRLF)
#define ERR_CMDNOTFOUND(nickname, command) (":localhost 421 " + nickname + " " + command + " :Unknown command" + CRLF)
#define ERR_NOORIGIN(nickname) (":localhost 409 " + nickname + " :No origin specified" + CRLF)
#define ERR_NOSUCHSERVER(nickname, targetserver) (":localhost 402 " + nickname + " " + targetserver + " :No such server" + CRLF)
#define ERR_UNKNOWNMODE(c, channel) (":localhost 472 " + c + " :is unknown mode char to me for " + channel + CRLF)
#endif // SERVERRESPONSES_HPP
