#include "bot/bot.hpp"

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/locale/locale_en.hpp"
#include "bot/commands.hpp"
#include "common/utility.hpp"

namespace kb {

void Bot::Bot::onAutocomplete(const dpp::autocomplete_t& event)
{
    const dpp::guild& guild = event.command.get_guild();
    std::string value;
    const LogMessageFunction logMessage = [&event, &guild, &value](const std::string& message)
    {
        return fmt::format(
            "\"{}\" / \"{}\": Autocomplete for /{} \"{}\": {}",
            guild.name, event.command.usr.format_username(),
            event.name, value, message
        );
    };

    std::lock_guard lock(m_mutex);
    updateInfoProcessedInteractions(guild.id);

    const dpp::command_option& option = event.options[0];
    if (option.name == CommandsConst::Seek::TimestampChapter::Name)
    {
        value = std::get<std::string>(option.value);        
        PlayerEntry playerEntry = m_players.find(guild.id);
        if (playerEntry == m_players.end())
        {
            interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply));
            m_logger.info(logMessage("Not in voice channel"));
            return;
        }

        Session session = playerEntry->second.session();
        if (!session.playingVideo)
        {
            interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply));
            m_logger.info(logMessage("Nothing is playing"));
            return;
        }

        if (session.playingVideo->video.chapters().empty())
        {
            interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply));
            m_logger.info(logMessage(fmt::format("Video \"{}\" has no chapters", session.playingVideo->video.title())));
            return;
        }

        if (value.empty() || value.find_first_not_of(' ') == std::string::npos)
        {
            dpp::interaction_response response(dpp::ir_autocomplete_reply);
            for (size_t index = 0, size = session.playingVideo->video.chapters().size(); index < size && index < 25; ++index)
            {
                const Youtube::Video::Chapter& chapter = session.playingVideo->video.chapters()[index];
                std::string firstPart = fmt::format("{}: ", chapter.number);
                std::string secondPart = fmt::format(" [{}]", Utility::NiceString(chapter.timestamp));
                response.add_autocomplete_choice(dpp::command_option_choice(fmt::format(
                    "{}\"{}\"{}",
                    firstPart,
                    Utility::Truncate(chapter.name, 100 - 2 - firstPart.length() - secondPart.length()),
                    secondPart
                ), chapter.name));
            }

            interaction_response_create(event.command.id, event.command.token, response);
            m_logger.info(logMessage("Displaying all chapters"));
            return;
        }

        dpp::interaction_response response(dpp::ir_autocomplete_reply);
        for (size_t index = 0, size = session.playingVideo->video.chapters().size(); index < size && response.autocomplete_choices.size() < 25; ++index)
        {
            const Youtube::Video::Chapter& chapter = session.playingVideo->video.chapters()[index];
            if (Utility::CaseInsensitiveStringContains(chapter.name, value))
            {
                std::string firstPart = fmt::format("{}: ", chapter.number);
                std::string secondPart = fmt::format(" [{}]", Utility::NiceString(chapter.timestamp));
                response.add_autocomplete_choice(dpp::command_option_choice(fmt::format(
                    "{}\"{}\"{}",
                    firstPart,
                    Utility::Truncate(chapter.name, 100 - 2 - firstPart.length() - secondPart.length()),
                    secondPart
                ), chapter.name));
            }
        }

        interaction_response_create(event.command.id, event.command.token, response);
        m_logger.info(logMessage(fmt::format(
            "Displaying {} chapter{}",
            response.autocomplete_choices.size(),
            LocaleEn::Cardinal(response.autocomplete_choices.size())
        )));
        return;
    }

    interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply));
    m_logger.error(logMessage(fmt::format("Unknown option: \"{}\"", option.name)));
}

} // namespace kb
