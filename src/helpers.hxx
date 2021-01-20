#ifndef HELPERS_HXX_
#define HELPERS_HXX_

#include <vector>
#include <string>
#include <sstream>

//void commaSeperatedStringToVector(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = ",");

//--------------------------------------------------------------------------------
 static void commaSeperatedStringToVector(const std::string& inputString, std::vector<std::string>& outputVector, const std::string& delimiter = ",")
{
  outputVector.clear();
  // Skip delimiters at beginning.
  std::string::size_type lastPos = inputString.find_first_not_of(delimiter, 0);

  // Find first non-delimiter.
  std::string::size_type pos = inputString.find_first_of(delimiter, lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos) {
    // Found a token, add it to the vector.
    outputVector.push_back(inputString.substr(lastPos, pos - lastPos));

    // Skip delimiters.
    lastPos = inputString.find_first_not_of(delimiter, pos);

    // Find next non-delimiter.
    pos = inputString.find_first_of(delimiter, lastPos);
  }
}

//--------------------------------------------------------------------------------

 static std::string commaSeperateVector(const std::vector<std::string>& inputVector, const char* const delimiter = ",")
 {
   switch (inputVector.size())
   {
   case 0:
     return "";
   case 1:
     return inputVector[0];
   default:
     std::ostringstream os;
     std::copy(inputVector.begin(), inputVector.end() - 1, std::ostream_iterator<std::string>(os, delimiter));
     os << *inputVector.rbegin();
     return os.str();
   }
 }

//--------------------------------------------------------------------------------

#endif