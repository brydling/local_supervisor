#ifndef TOKENIZESTRING_H
#define TOKENIZESTRING_H

#include <vector>
#include <string>

/* Takes a string consisting of tokens separated by a separator character and divides that
   string into separate sub-strings. Even the last token before the end of the string must
   end with the separator character to be included in the result.
   ARGS:
      line (in):
         The string to be tokenized
      separator (in):
         The separator character
   RETVAL:
      A vector of tokens
*/
std::vector<std::string> TokenizeString(std::string line, char separator);

#endif