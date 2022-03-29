//
// Created by maros on 22/03/2022.
//

#include "bot.hpp"

#include <filesystem>
#include <spdlog/fmt/fmt.h>

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
    if (!file.is_open())
        return;
    file.seekg(0, std::ios::end);
    int64_t length = file.tellg();
    file.seekg(0, std::ios::beg);
    auto buffer = new char[length + 1];
    file.read(buffer, length);
    buffer[length] = 0;

    try {
        auto root = nlohmann::json::parse(buffer);
        for (const auto &entry: root) {
            dpp::snowflake guild = entry["guild"];
            this->m_data[guild] = entry["settings"];
        }
    } catch (const std::exception &x) {
        spdlog::error("Can't read json settings.");
    }
}

rgntware::Module::Module(const std::shared_ptr<dpp::cluster> &mCluster) : m_cluster(mCluster) {}

rgntware::Bot::Bot(const std::string &mToken)
        : m_token(mToken) {
    this->m_cluster = std::make_shared<dpp::cluster>(this->m_token, dpp::i_guild_members
                                                                    | dpp::i_message_content
                                                                    | dpp::i_default_intents

    );

//    this->m_cluster->on_log([this](const dpp::log_t & event) {
//        switch (event.severity) {
//            case dpp::ll_trace:
//                spdlog::trace(fmt::format("{}", event.message));
//                break;
//            case dpp::ll_debug:
//                spdlog::debug(fmt::format("{}", event.message));
//                break;
//            case dpp::ll_info:
//                spdlog::info(fmt::format("{}", event.message));
//                break;
//            case dpp::ll_warning:
//                spdlog::warn(fmt::format("{}", event.message));
//                break;
//            case dpp::ll_error:
//                spdlog::error(fmt::format("{}", event.message));
//                break;
//            case dpp::ll_critical:
//            default:
//                spdlog::critical(fmt::format("{}", event.message));
//                break;
//        }
//    });
}

void rgntware::Bot::init() {
    for (auto &module: this->m_modules) {
        module->init();
    }
    try {
        this->m_cluster->start();
    } catch (const std::exception &x) {
        spdlog::error("error");
    }
}

void rgntware::Bot::term() {
    for (auto &module: this->m_modules) {
        module->term();
    }
}

void rgntware::Bot::add_module(const std::shared_ptr<rgntware::Module> &module) {
    this->m_modules.push_back(module);
}


