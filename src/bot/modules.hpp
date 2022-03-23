//
// Created by maros on 22/03/2022.
//

#ifndef RGNTWARE_MODULES_HPP
#define RGNTWARE_MODULES_HPP

#include <spdlog/fmt/fmt.h>
#include "bot.hpp"

namespace rgntware {

    namespace tickets {

        struct Ticket {
        public:
            std::string topic;
            dpp::snowflake owner;
            dpp::snowflake guild;

            dpp::snowflake channel = 0;
        };

        /**
         * Ticket Module
         */
        class Tickets : public rgntware::Module {
        private:
            std::unordered_map<dpp::snowflake, std::pair<std::mutex, std::unordered_map<dpp::snowflake, Ticket>>> tickets;
            std::mutex ticketsLock;

        public:
            Tickets(const std::shared_ptr<dpp::cluster> &mCluster);

        public:
            void init() override;
            void term() override;

            void create_ticket(const Ticket& ticket);
            void delete_ticket(const dpp::snowflake& guild, const dpp::snowflake& owner);
        };
    }

    namespace autorole {
        class AutoRole : public rgntware::Module {
        public:
            AutoRole(const std::shared_ptr<dpp::cluster> &mCluster);

        public:
            void init() override;
            void term() override;
        };
    }
}


#endif //RGNTWARE_MODULES_HPP
