//
// Created by maros on 22/03/2022.
//

#ifndef RGNTWARE_BOT_HPP
#define RGNTWARE_BOT_HPP

#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>

#include <dpp/dpp.h>
#include <cxxopts.hpp>
#include <spdlog/spdlog.h>

namespace rgntware {

    /**
     * Represents bot memory.
     */
    class GuildMemory {
    private:
        std::unordered_map<dpp::snowflake, nlohmann::json> m_data;
    public:
        nlohmann::json &operator[](const dpp::snowflake &id);

        void store(const char* folder, const char* file);
        void load(const char* folder, const char* file);
    };

    /**
     * Represents single bot module.
     */
    class Module {
    protected:
        std::shared_ptr<dpp::cluster> m_cluster;
        GuildMemory m_guildMemory;

    public:
        Module(const std::shared_ptr<dpp::cluster> &mCluster);

    public:
        virtual void init() = 0;
        virtual void term() = 0;
    };

    /**
     * Represents bot.
     */
    class Bot {
    public:
        std::string m_token;
        std::shared_ptr<dpp::cluster> m_cluster;
        std::vector<std::shared_ptr<rgntware::Module>> m_modules;

    public:
        Bot(const std::string &mToken);
        void add_module(const std::shared_ptr<rgntware::Module>& module);
        void init();
        void term();
    };

}

#endif //RGNTWARE_BOT_HPP
