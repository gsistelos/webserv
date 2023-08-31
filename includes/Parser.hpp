#pragma once

#include <fstream>
#include <vector>

class Parser {
   public:
    static void readFile(const std::string& filename, std::string& buf);
    static void extractWord(std::string& str, std::string& buf);
};
