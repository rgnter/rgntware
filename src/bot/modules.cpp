//
// Created by maros on 22/03/2022.
//

#include <variant>
#include "modules.hpp"

static const std::string TICKET_TOPIC  = "rw::ticket::topic";
static const std::string TICKET_CREATE = "rw::ticket::create";
static const std::string TICKET_DELETE = "rw::ticket::delete";
static const std::string TICKET_MODAL  = "rw::ticket::modal";

rgntware::tickets::Tickets::Tickets(const std::shared_ptr<dpp::cluster> &mCluster)
        : Module(mCluster) {}


void rgntware::tickets::Tickets::init() {
    this->m_cluster->on_ready([this](const dpp::ready_t &event) {
        if (dpp::run_once<struct tickets_register_commands>()) {
            {
                auto ticketInteractions = dpp::slashcommand("ticket_interactions", "Shows ticket interaction buttons.",
                                                            this->m_cluster->me.id);

                for (const auto &guild: this->m_cluster->current_user_get_guilds_sync()) {
                    this->m_cluster->guild_command_create(ticketInteractions, guild.first);
                }
            }
        }
    });


    this->m_cluster->on_interaction_create([this](const dpp::interaction_create_t &event) {
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
        }
    });

    this->m_cluster->on_button_click([this](const dpp::button_click_t& event) {
        if(event.custom_id == TICKET_CREATE) {
            if(this->tickets.contains(event.command.usr.id)) {
                event.reply(dpp::message("You already have an active ticket.").set_flags(dpp::m_ephemeral));
                return;
            }
            dpp::interaction_modal_response modal(TICKET_MODAL, "Please pick a ticket topic.");
            modal.add_component(
                    dpp::component().
                            set_label("Ticket Topic").
                            set_id(TICKET_TOPIC).
                            set_type(dpp::cot_text).
                            set_placeholder(fmt::format("{}'s ticket", event.command.usr.username)).
                            set_min_length(5).
                            set_max_length(32).
                            set_text_style(dpp::text_short)
            );
            event.dialog(modal);
            return;
        }
        if(event.custom_id == TICKET_DELETE) {
            if(!this->tickets.contains(event.command.usr.id)) {
                event.reply(dpp::message("You do not have an active ticket.").set_flags(dpp::m_ephemeral));
                return;
            }
            event.reply(dpp::message("Sure thing.").set_flags(dpp::m_ephemeral));
            this->delete_ticket(event.command.usr.id);
            return;
        }
    });

    this->m_cluster->on_form_submit([this](const dpp::form_submit_t& event){
        if(event.custom_id == TICKET_MODAL) {
            std::string topic;
            for (const auto &cp : event.components) {
                if(cp.custom_id == TICKET_TOPIC) {
                    topic = std::get<std::string>(cp.value);
                }
            }
            auto ticket = Ticket{topic, event.command.usr.id};
            this->create_ticket(ticket);
        }
    });
}

void rgntware::tickets::Tickets::term() {

}


void rgntware::tickets::Tickets::create_ticket(const rgntware::tickets::Ticket &ticket) {
    std::lock_guard<std::mutex> lock(this->ticketsLock);
    this->tickets[ticket.owner] = ticket;

    auto ticketChannel = dpp::channel()
            .set_name(ticket.topic);
    this->m_cluster->channel_create(ticketChannel, [this, ticketChannel, ticket](const dpp::confirmation_callback_t& ev) {
        this->tickets[ticket.owner].channel = ticketChannel.id;
        spdlog::error("im here");
    });
}

void rgntware::tickets::Tickets::delete_ticket(const dpp::snowflake &owner) {
    std::lock_guard<std::mutex> lock(this->ticketsLock);
    auto ticket = this->tickets[owner];
    if(ticket.channel == 0)
        return;
    this->m_cluster->channel_delete(ticket.channel);
}