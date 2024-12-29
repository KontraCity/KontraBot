#include "bot/bot.hpp"

namespace kb {

void Bot::Bot::onSelectClick(const dpp::select_click_t& event)
{
    const dpp::guild& guild = event.command.get_guild();
    const LogMessageFunction logMessage = [event, guild](const std::string& message)
    {
        return fmt::format(
            "\"{}\" / \"{}\": [{}/{}]: {}",
            guild.name, event.command.usr.format_username(),
            event.custom_id, event.values[0], message
        );
    };

    std::lock_guard lock(m_mutex);
    Info info = updateInfoProcessedInteractions(guild.id);

    if (playerControlsLocked(guild, event.command.usr.id))
    {
        event.reply(info.settings().locale->onlyUsersWithMeCanControlPlayer());
        m_logger.info(logMessage("Player controls are locked for user"));
        return;
    }

    Signal signal(event.values[0]);
    switch (signal.type())
    {
        case Signal::Type::PlayVideo:
        case Signal::Type::PlayPlaylist:
        {
            std::thread([this, event, logMessage, signal]()
            {
                event.thinking();
                event.edit_original_response(addItem(event.from, event.command, signal.data(), logMessage, true));
            }).detach();
            break;
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
            m_logger.error(logMessage("Unknown select option"));
            break;
        }
    }

    if (m_ephemeralTokens.find(event.command.msg.id) != m_ephemeralTokens.end())
    {
        dpp::message message = event.command.msg;
        dpp::component& selectMenu = message.components[0].components[0];
        for (const dpp::select_option& option : selectMenu.options)
        {
            if (option.value == event.values[0])
            {
                selectMenu.placeholder = option.label;
                break;
            }
        }
        selectMenu.disabled = true;

        interaction_followup_edit(m_ephemeralTokens[message.id], message);
        m_ephemeralTokens.erase(event.command.msg.id);
    }
}

} // namespace kb
