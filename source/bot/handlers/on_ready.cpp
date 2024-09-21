#include "bot/bot.hpp"

namespace kc {

void Bot::Bot::onReady(const dpp::ready_t& event)
{
    if (dpp::run_once<struct ReadyMessage>())
    {
        // Start presence thread
        m_presenceThread = std::thread(&Bot::presenceFunction, this);

        global_commands_get([this](const dpp::confirmation_callback_t& event)
        {
            if (event.is_error())
            {
                m_logger.critical("Couldn't get bot commands");
                return;
            }

            try
            {
                const dpp::slashcommand_map& commands = std::get<dpp::slashcommand_map>(event.value);
                Commands::Instance->parse(commands);
            }
            catch (const std::runtime_error& error)
            {
                m_logger.critical("Couldn't parse bot commands: {}", error.what());
                return;
            }

            m_logger.info("Ready: logged in as {}", me.format_username());
            current_user_get_guilds([this](const dpp::confirmation_callback_t& event)
            {
                if (event.is_error())
                {
                    m_logger.error("Couldn't get bot guilds");
                    return;
                }

                const dpp::guild_map& guilds = std::get<dpp::guild_map>(event.value);
                if (guilds.empty())
                {
                    m_logger.info("No guilds are served");
                    return;
                }
                else if (guilds.size() == 1)
                {
                    m_logger.info(
                        "Serving 1 guild: \"{}\" [{}]",
                        guilds.cbegin()->second.name,
                        static_cast<uint64_t>(guilds.cbegin()->second.id)
                    );
                    return;
                }

                m_logger.info("Serving {} guilds:", guilds.size());
                for (const auto& guild : guilds)
                    m_logger.info("    \"{}\" [{}]", guild.second.name, static_cast<uint64_t>(guild.second.id));
            });
        });
    }
}

} // namespace kc
