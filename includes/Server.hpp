#pragma once

#include "ConfigBlock.hpp"
#include "Fd.hpp"

/*
 * Server class store configurations
 * and accepts incoming connections
 **/
class Server : public Fd {
   public:
    Server(t_listen hostPort);
    ~Server();

    const ConfigBlock& getConfig(const std::string& serverName);
    void push_back(ConfigBlock& configBlock);

    void handlePollin(int index);
    void handlePollout(int index);

   private:
    std::vector<ConfigBlock> _configBlocks;
};
