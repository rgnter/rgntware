#include <cstdio>

#include "bot/bot.hpp"
#include "bot/modules.hpp"

#include <cxxopts.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char **argv) {

    cxxopts::Options options{"rgntware", "rgntware is a shard."};
    options.add_options()
            ("t,token", "discord shard token", cxxopts::value<std::string>());
    auto result = options.parse(argc, argv);
    if (result["token"].count() == 0) {
        spdlog::error("You must provide discord token via argument -t or --token.");
        return 0;
    }

    bool shouldRun = true;
    {
        rgntware::Bot bot(result["token"]
                                  .as<std::string>());
        {
            bot.add_module(
                    std::make_shared<rgntware::tickets::Tickets>(bot.m_cluster)
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
