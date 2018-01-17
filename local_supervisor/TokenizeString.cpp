#include "stdafx.h"
#include "TokenizeString.h"

#include <vector>
#include <string>

std::vector<std::string> TokenizeString(std::string line, char separator) {
   std::vector<std::string> tokens;

   size_t oldPos = 0;
   size_t newPos;
   do {
      newPos = line.find(separator, oldPos);
      if(newPos != std::string::npos) {	// If we found a separator character
         std::string token = line.substr(oldPos, newPos-oldPos);
         oldPos = newPos+1;

         tokens.push_back(token);
      }
      else {											// We reached the end of the string,
         if (oldPos < line.length())					// if there are characters after the last separator,
            tokens.push_back(line.substr(oldPos));	// add them.
      }
   } while(newPos != std::string::npos);

   return tokens;
}