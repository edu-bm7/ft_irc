#include "helper_functions.hpp"

std::string trim(const std::string& s)
{
	size_t start = s.find_first_not_of(" \t\r\v\n");
	size_t end = s.find_last_not_of(" \t\r\n\v");
	return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

bool isValidChannelName(const std::string& name)
{
	// Channel names must start with '&', '#', or '!'
	if (name.empty() || (name[0] != '#' && name[0] != '&')) {
		return false;
	}
	return true;
}

std::vector< std::string > split(const std::string& s, char delimiter)
{
	std::vector< std::string > tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		if (!token.empty()) {
			tokens.push_back(token);
		}
	}
	return tokens;
}

// Function to split a string by commas
std::vector< std::string > splitByComma(const std::string& s)
{
	std::vector< std::string > result;
	size_t start = 0;
	size_t comma_pos = s.find(',');

	while (comma_pos != std::string::npos) {
		std::string token = s.substr(start, comma_pos - start);
		result.push_back(trim(token));
		start = comma_pos + 1;
		comma_pos = s.find(',', start);
	}

	// Add the last token
	std::string token = s.substr(start);
	result.push_back(trim(token));

	return result;
}
