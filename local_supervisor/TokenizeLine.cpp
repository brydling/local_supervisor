#include "stdafx.h"
#include "TokenizeLine.h"

#include <vector>
#include <string>

std::vector<std::string> TokenizeLine(std::string line, char separator) {
	std::vector<std::string> tokens;

	size_t oldPos = 0;
	size_t newPos;
	do {
		newPos = line.find(separator, oldPos);
		if(newPos != std::string::npos) {
			std::string token = line.substr(oldPos, newPos-oldPos);
			oldPos = newPos+1;

			tokens.push_back(token);
		}
	} while(newPos != std::string::npos);

	return tokens;
}