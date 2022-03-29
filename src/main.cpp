#include <cstdio>

#include <spdlog/fmt/fmt.h>
#include "bot/bot.hpp"
#include "bot/modules.hpp"

#include <fstream>
#include <cxxopts.hpp>


std::string retrieve_token(cxxopts::ParseResult& cli) {
    if(cli.count("token"))
        return cli["token"].as<std::string>();


    std::ifstream token_store("token", std::ios::binary);
    if(token_store.fail())
        return "";
    std::streamsize file_length;
    {
        token_store.seekg(0, std::ios::end);
        file_length = token_store.tellg();
        token_store.seekg(0, std::ios::beg);
    }

    auto token_buffer = new char[file_length+1];
    token_store.read(token_buffer, file_length);
    token_buffer[file_length] = 0;
    std::string token(token_buffer);
    delete[] token_buffer;
    token_store.close();

    return token;
}

int main(int argc, char **argv) {
    cxxopts::Options options{"rgntware", "rgntware is a discord bot."};
    options.add_options()
            ("t,token", "discord shard token", cxxopts::value<std::string>());
    auto cli = options.parse(argc, argv);
    const auto token = retrieve_token(cli);



    if(token.empty()) {
        spdlog::error("Discord API token must be provided either with program argument --token, or with file called \"token\" which contains it.");
        return 0;
    }

    bool shouldRun = true;
    {
        rgntware::Bot bot(token);
        {
            bot.add_module(
                    std::make_shared<rgntware::tickets::Tickets>(bot.m_cluster)
            );
            bot.add_module(
                    std::make_shared<rgntware::autorole::AutoRole>(bot.m_cluster)
            );
        }
        spdlog::info("Modules registered.");
        bot.init();
        spdlog::info("Initialized.");
        while (shouldRun) {
            std::string cmd;
            std::getline(std::cin, cmd);

            if (cmd == "stop")
                shouldRun = false;
        }
        bot.term();
        spdlog::info("Terminated.");
    }
}
