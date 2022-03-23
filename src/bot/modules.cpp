//
// Created by maros on 22/03/2022.
//

#include <variant>
#include "modules.hpp"

static const std::string TICKET_TOPIC = "rw::ticket::topic";
static const std::string TICKET_CREATE = "rw::ticket::create";
static const std::string TICKET_DELETE = "rw::ticket::delete";
static const std::string TICKET_MODAL = "rw::ticket::modal";

rgntware::tickets::Tickets::Tickets(const std::shared_ptr<dpp::cluster> &mCluster)
        : Module(mCluster) {}


void rgntware::tickets::Tickets::init() {
    this->m_guildMemory.load("data/tickets", "settings.json");
    // commands
    this->m_cluster->on_ready([this](const dpp::ready_t &event) {
        if (dpp::run_once<struct tickets_register_commands>()) {
            {
                auto ticketCategory = dpp::slashcommand("set_ticket_category", "Sets ticket category",
                                                        this->m_cluster->me.id)
                        .add_option(dpp::command_option(dpp::co_channel, "category", "category"));
                auto ticketInteractions = dpp::slashcommand("ticket_interactions", "Shows ticket interaction buttons.",
                                                            this->m_cluster->me.id);

                for (const auto &guild: this->m_cluster->current_user_get_guilds_sync()) {
                    this->m_cluster->guild_command_create(ticketInteractions, guild.first);
                    this->m_cluster->guild_command_create(ticketCategory, guild.first);
                }
            }
        }
    });

    // command interactions
    this->m_cluster->on_interaction_create([this](const dpp::interaction_create_t &event) {
        if (event.command.get_command_name() == "set_ticket_category") {
            auto channelID = std::get<dpp::snowflake>(event.command.get_command_interaction().options[0].value);
            this->m_guildMemory[event.command.guild_id]["ticket_category"] = channelID;
            event.reply(dpp::message("Okay.").set_flags(dpp::m_ephemeral));
            return;
        }
        if (event.command.get_command_name() == "ticket_interactions") {
            event.reply(dpp::message("Alright.").set_flags(dpp::m_ephemeral));

            const auto panel = dpp::message(event.command.channel_id, "")
                    .add_embed(dpp::embed()
                                       .set_color(0xbada55)
                                       .set_description(
                                               "You can create your ticket by pressing the button below."))
                    .add_component(dpp::component()
                                           .add_component(dpp::component()
                                                                  .set_label("Create ticket")
                                                                  .set_type(dpp::cot_button)
                                                                  .set_style(dpp::cos_success)
                                                                  .set_id(TICKET_CREATE)));
            this->m_cluster->message_create(panel);
            return;
        }
    });

    // button interactions
    this->m_cluster->on_button_click([this](const dpp::button_click_t &event) {
        if (event.custom_id == TICKET_CREATE) {
            if (this->tickets.contains(event.command.usr.id)) {
                event.reply(dpp::message("You already have an active ticket.").set_flags(dpp::m_ephemeral));
                return;
            }
            dpp::interaction_modal_response modal(TICKET_MODAL, "Please pick a ticket topic.");

            auto placeholder = fmt::format("{}'s ticket", event.command.usr.username);
            modal.add_component(
                    dpp::component().
                            set_label("Ticket Topic").
                            set_id(TICKET_TOPIC).
                            set_type(dpp::cot_text).
                            set_placeholder(placeholder).
                            set_min_length(5).
                            set_max_length(32).
                            set_text_style(dpp::text_short)
            ).set_custom_id(TICKET_MODAL);
            event.dialog(modal);
            return;
        }
        if (event.custom_id == TICKET_DELETE) {
            if (!this->tickets[event.command.guild_id].second.contains(event.command.usr.id)) {
                event.reply(dpp::message("You do not have an active ticket.").set_flags(dpp::m_ephemeral));
                return;
            }
            event.reply(dpp::message("Sure thing.").set_flags(dpp::m_ephemeral));
            this->delete_ticket(event.command.guild_id, event.command.usr.id);
            return;
        }
    });

    // modal interactions
    this->m_cluster->on_form_submit([this](const dpp::form_submit_t &event) {
        if (event.custom_id == TICKET_MODAL) {
            std::string topic;
            bool set = false;

            auto findIdentifiedComponent = [&topic, &set](const dpp::component &component) {
                if (component.custom_id == TICKET_TOPIC) {
                    topic = std::get<std::string>(component.value);
                    set = true;
                }
            };

            auto recursiveQueryComponents = [&findIdentifiedComponent](const std::vector<dpp::component> &components) {
                for (const auto &item: components) {
                    if (item.type == dpp::cot_action_row)
                        for (const auto &item: item.components) {
                            findIdentifiedComponent(item);
                        }
                    else
                        findIdentifiedComponent(item);
                }
            };
            recursiveQueryComponents(event.components);

            if (!set)
                return;
            auto ticket = Ticket{topic, event.command.usr.id, event.command.guild_id};
            event.reply(dpp::message("Submitted your ticket.").set_flags(dpp::m_ephemeral));
            this->create_ticket(ticket);
        }
    });
}

void rgntware::tickets::Tickets::term() {
    this->m_guildMemory.store("data/tickets", "settings.json");
}


void rgntware::tickets::Tickets::create_ticket(const rgntware::tickets::Ticket &ticket) {
    auto &guildData = this->tickets[ticket.guild];
    std::lock_guard<std::mutex> semaphoreLock(guildData.first);
    {
        dpp::snowflake ticketCategory = this->m_guildMemory[ticket.guild]["ticket_category"];
        auto channel = dpp::channel()
                .set_name(ticket.topic)
                .set_topic(fmt::format("{}'s ticket: {}", dpp::find_user(ticket.owner)->username, ticket.topic));
        if (ticketCategory != 0)
            channel.set_parent_id(ticketCategory);
        channel.guild_id = ticket.guild;

        this->m_cluster->channel_create(channel, [this, ticket](const dpp::confirmation_callback_t &event) {
            const auto channel = std::get<dpp::channel>(event.value);
            // update ticket channel id
            this->tickets[channel.guild_id].second[ticket.owner].channel = channel.id;

            // send interaction panel
            this->m_cluster->message_create(dpp::message(channel.id, "")
                                                    .add_embed(dpp::embed()
                                                                       .set_color(0x777777)
                                                                       .set_description("Somebody from the support team, will be with you shortly."))
                                                    .add_component(dpp::component()
                                                                           .add_component(dpp::component()
                                                                                                  .set_label("Close ticket")
                                                                                                  .set_type(dpp::cot_button)
                                                                                                  .set_style(dpp::cos_danger)
                                                                                                  .set_id(TICKET_DELETE))));
        });

        guildData.second[ticket.owner] = ticket;
    }
}

void rgntware::tickets::Tickets::delete_ticket(const dpp::snowflake &guild, const dpp::snowflake &owner) {
    auto &guildData = this->tickets[guild];
    std::lock_guard<std::mutex> semaphoreLock(guildData.first);
    {
        auto ticket = guildData.second[owner];
        this->m_cluster->channel_delete(ticket.channel);
    }
}
