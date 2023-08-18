#pragma once

#include <fstream>
#include <vector>

class Parser {
   public:
    static std::string readFile(const std::string& filename);
    static std::string extractWord(std::string& str);
};
