#ifndef HELPER_FUNCTIONS_HPP
#define HELPER_FUNCTIONS_HPP

#include <sstream>
#include <string>
#include <vector>

std::string trim(const std::string& s);
bool isValidChannelName(const std::string& name);
std::vector< std::string > split(const std::string& s, char delimiter);
std::vector< std::string > splitByComma(const std::string& s);

#endif // HELPER_FUNCTIONS_HPP
