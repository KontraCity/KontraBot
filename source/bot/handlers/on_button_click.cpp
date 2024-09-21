#include "bot/bot.hpp"

namespace kc {

void Bot::Bot::onButtonClick(const dpp::button_click_t& event)
{
    const dpp::guild& guild = event.command.get_guild();
    const LogMessageFunction logMessage = [event, guild](const std::string& message)
    {
        return fmt::format(
            "\"{}\" / \"{}\": [{}]: {}",
            guild.name, event.command.usr.format_username(), event.custom_id, message
        );
    };

    std::lock_guard lock(m_mutex);
    updatePlayerTextChannelId(guild.id, event.command.channel_id);
    Info info = updateInfoProcessedInteractions(guild.id);

    const Signal signal(event.custom_id);
    switch (signal.type())
    {
        case Signal::Type::PlayVideo:
        case Signal::Type::PlayPlaylist:
        {
            if (playerControlsLocked(guild, event.command.usr.id))
            {
                m_logger.info(logMessage("Player controls are locked for user"));
                event.reply(info.settings().locale->onlyUsersWithMeCanControlPlayer());
                return;
            }

            event.reply(addItem(event.from, event.command, signal.data(), logMessage, info, true));
            break;
        }
        case Signal::Type::RelatedSearch:
        {
            std::thread([this, event, guild, logMessage, signal]()
            {
                try
                {
                    event.thinking(true);
                    Youtube::Results results = Youtube::Related(signal.data());

                    std::lock_guard lock(m_mutex);
                    event.edit_original_response(
                        Info(guild.id).settings().locale->search(results),
                        std::bind(&Bot::updateEphemeralToken, this, std::placeholders::_1, event.command.token)
                    );
                    m_logger.info(logMessage(fmt::format(
                        "Related for \"{}\": {} result{}",
                        signal.data(), results.size(), LocaleEn::Cardinal(results.size())
                    )));
                }
                catch (const std::runtime_error& error)
                {
                    std::lock_guard lock(m_mutex);
                    event.edit_original_response(Info(guild.id).settings().locale->unknownError());
                    m_logger.error(logMessage(fmt::format(
                        "Related for \"{}\": Runtime error: {}",
                        signal.data(), error.what()
                    )));
                }
            }).detach();
            return;
        }
        case Signal::Type::Unsupported:
        {
            event.reply(info.settings().locale->unsupportedButton());
            m_logger.info(logMessage("Unsupported button"));
            break;
        }
        default:
        {
            event.reply(info.settings().locale->unknownButton());
            m_logger.error(logMessage("Unknown button"));
            break;
        }
    }

    auto ephemeralTokenEntry = m_ephemeralTokens.find(event.command.msg.id);
    if (ephemeralTokenEntry == m_ephemeralTokens.end())
        return;

    dpp::message message = event.command.msg;
    for (dpp::component& component : message.components[0].components)
        component.disabled = true;

    interaction_followup_edit(ephemeralTokenEntry->second, message);
    m_ephemeralTokens.erase(event.command.msg.id);
}

} // namespace kc
