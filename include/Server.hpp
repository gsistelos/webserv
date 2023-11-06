#pragma once

#include "ConfigBlock.hpp"
#include "Fd.hpp"

/*
 * Server class store configurations
 * and accepts incoming connections
 **/
class Server : public Fd {
   public:
    Server(t_listen hostPort, ConfigBlock& configBlock);
    ~Server();
    bool operator==(const t_listen& hostPort) const;

    const ConfigBlock& getConfig(const std::string& serverName);

    void configToServerName(ConfigBlock& configBlock);
    void handlePollin(int index);
    void handlePollout(int index);

   private:
    t_listen _hostPort;
    ConfigBlock* _configDefault;
    std::map<std::string, ConfigBlock*> _configs;
};
