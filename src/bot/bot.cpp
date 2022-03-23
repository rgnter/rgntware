//
// Created by maros on 22/03/2022.
//

#include "bot.hpp"
#include <spdlog/fmt/fmt.h>
#include <filesystem>


nlohmann::json &rgntware::GuildMemory::operator[](const dpp::snowflake &id) {
    if (!this->m_data.contains(id))
        m_data[id] = nlohmann::json();
    return m_data[id];
}

void rgntware::GuildMemory::store(const char *folder,
                                  const char *fileName) {
    std::string path;
    path += folder;
    path += std::filesystem::path::preferred_separator;
    path += fileName;

    std::filesystem::create_directories(folder);
    std::ofstream file(path);

    nlohmann::json root{};
    for (const auto &guildData: this->m_data) {
        nlohmann::json entry;
        entry["guild"] = guildData.first;
        entry["settings"] = guildData.second;
        root += entry;
    }
    auto dump = root.dump();
    file.write(dump.data(), dump.size());
    file.close();
}

void rgntware::GuildMemory::load(const char *folder,
                                 const char *fileName) {
    std::string path;
    path += folder;
    path += std::filesystem::path::preferred_separator;
    path += fileName;

    std::ifstream file(path, std::ios::binary);
    if(!file.is_open())
        return;
    file.seekg(0, std::ios::end);
    int64_t length = file.tellg();
    file.seekg(0, std::ios::beg);
    auto buffer = new char[length+1];
    file.read(buffer, length);
    buffer[length] = 0;

    try {
        auto root = nlohmann::json::parse(buffer);
        for (const auto &entry : root) {
            dpp::snowflake guild = entry["guild"];
            this->m_data[guild] = entry["settings"];
        }
    } catch (const std::exception& x) {
        spdlog::error("Can't read json settings.");
    }
}

rgntware::Module::Module(const std::shared_ptr<dpp::cluster> &mCluster) : m_cluster(mCluster) {}

rgntware::Bot::Bot(const std::string &mToken)
        : m_token(mToken) {
    this->m_cluster = std::make_shared<dpp::cluster>(this->m_token, dpp::i_default_intents | dpp::i_message_content);
    this->m_cluster->on_message_create([](const dpp::message_create_t& event) {
        if(event.msg.content == "<3") {
            event.reply(":heart:²");
        }
    });
}

void rgntware::Bot::init() {
    for (auto &module : this->m_modules) {
        module->init();
    }
    this->m_cluster->start();
}

void rgntware::Bot::term() {
    for (auto &module : this->m_modules) {
        module->term();
    }
}

void rgntware::Bot::add_module(const std::shared_ptr<rgntware::Module>& module) {
    this->m_modules.push_back(module);
}


