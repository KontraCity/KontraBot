#include "bot/commands.hpp"
using namespace kb::Bot::CommandsConst;

// STL modules
#include <stdexcept>

// Library {fmt}
#include <fmt/format.h>

namespace kb {

/*
*   std::make_unique() needs public constructor, but the Youtube::Client class uses singleton pattern.
*   This is why operator new is used instead.
*/
const std::unique_ptr<Bot::Commands> Bot::Commands::Instance(new Bot::Commands);

std::vector<dpp::slashcommand> Bot::Commands::GetCommands(dpp::snowflake botId)
{
    std::vector<dpp::slashcommand> commands;

    /* /help */
    commands.push_back(dpp::slashcommand(Help::Name, Help::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Help::Name, Russian::Help::Description));

    /* /session */
    commands.push_back(dpp::slashcommand(Session::Name, Session::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Session::Name, Russian::Session::Description));

    /* /settings */
    commands.push_back(dpp::slashcommand(Settings::Name, Settings::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Settings::Name, Russian::Settings::Description));

    /* /stats */
    commands.push_back(dpp::slashcommand(Stats::Name, Stats::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Stats::Name, Russian::Stats::Description));

    /* /set language, /set timeout, /set change-status */
    commands.push_back(dpp::slashcommand(Set::Name, "-", botId)
        .add_localization(Russian::LocaleName, Russian::Set::Name, "-")
        .add_option(dpp::command_option(dpp::co_sub_command, Set::Language::Name, Set::Language::Description)
            .add_localization(Russian::LocaleName, Russian::Set::Language::Name, Russian::Set::Language::Description)
            .add_option(dpp::command_option(dpp::co_string, Set::Language::Language::Name, Set::Language::Language::Description, true)
                .add_localization(Russian::LocaleName, Russian::Set::Language::Language::Name, Russian::Set::Language::Language::Description)
                .add_choice(dpp::command_option_choice(Set::Language::Language::Label, std::string(Set::Language::Language::Id)))
                .add_choice(dpp::command_option_choice(Russian::Set::Language::Language::Label, std::string(Russian::Set::Language::Language::Id)))))
        .add_option(dpp::command_option(dpp::co_sub_command, Set::Timeout::Name, Set::Timeout::Description)
            .add_localization(Russian::LocaleName, Russian::Set::Timeout::Name, Russian::Set::Timeout::Description)
            .add_option(dpp::command_option(dpp::co_integer, Set::Timeout::Duration::Name, Set::Timeout::Duration::Description, true)
                .add_localization(Russian::LocaleName, Russian::Set::Timeout::Duration::Name, Russian::Set::Timeout::Duration::Description)))
        .add_option(dpp::command_option(dpp::co_sub_command, Set::ChangeStatus::Name, Set::ChangeStatus::Description)
           .add_localization(Russian::LocaleName, Russian::Set::ChangeStatus::Name, Russian::Set::ChangeStatus::Description)
           .add_option(dpp::command_option(dpp::co_string, Set::ChangeStatus::Change::Name, Set::ChangeStatus::Change::Description, true)
               .add_localization(Russian::LocaleName, Russian::Set::ChangeStatus::Change::Name, Russian::Set::ChangeStatus::Change::Description)
               .add_choice(dpp::command_option_choice(Set::ChangeStatus::Change::Yes::Label, std::string(Set::ChangeStatus::Change::Yes::Id))
                   .add_localization(Russian::LocaleName, Russian::Set::ChangeStatus::Change::Yes::Label))
               .add_choice(dpp::command_option_choice(Set::ChangeStatus::Change::No::Label, std::string(Set::ChangeStatus::Change::No::Id))
                   .add_localization(Russian::LocaleName, Russian::Set::ChangeStatus::Change::No::Label)))));

    /* /join */
    commands.push_back(dpp::slashcommand(Join::Name, Join::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Join::Name, Russian::Join::Description));

    /* /leave */
    commands.push_back(dpp::slashcommand(Leave::Name, Leave::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Leave::Name, Russian::Leave::Description));

    /* /play <what> */
    commands.push_back(dpp::slashcommand(Play::Name, Play::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Play::Name, Russian::Play::Description)
        .add_option(dpp::command_option(dpp::co_string, Play::What::Name, Play::What::Description, true)
            .add_localization(Russian::LocaleName, Russian::Play::What::Name, Russian::Play::What::Description)));

    /* /pause */
    commands.push_back(dpp::slashcommand(Pause::Name, Pause::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Pause::Name, Russian::Pause::Description));

    /* /seek <timestamp-chapter> */
    commands.push_back(dpp::slashcommand(Seek::Name, Seek::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Seek::Name, Russian::Seek::Description)
        .add_option(dpp::command_option(dpp::co_string, Seek::TimestampChapter::Name, Seek::TimestampChapter::Description, true)
            .add_localization(Russian::LocaleName, Russian::Seek::TimestampChapter::Name, Russian::Seek::TimestampChapter::Description)
            .set_auto_complete(true)));

    /* /shuffle */
    commands.push_back(dpp::slashcommand(Shuffle::Name, Shuffle::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Shuffle::Name, Russian::Shuffle::Description));

    /* /skip video, /skip playlist */
    commands.push_back(dpp::slashcommand(Skip::Name, "-", botId)
        .add_localization(Russian::LocaleName, Russian::Skip::Name, "-")
        .add_option(dpp::command_option(dpp::co_sub_command, Skip::Video::Name, Skip::Video::Description)
            .add_localization(Russian::LocaleName, Russian::Skip::Video::Name, Russian::Skip::Video::Description))
        .add_option(dpp::command_option(dpp::co_sub_command, Skip::Playlist::Name, Skip::Playlist::Description)
            .add_localization(Russian::LocaleName, Russian::Skip::Playlist::Name, Russian::Skip::Playlist::Description)));

    /* /clear */
    commands.push_back(dpp::slashcommand(Clear::Name, Clear::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Clear::Name, Russian::Clear::Description));

    /* /stop */
    commands.push_back(dpp::slashcommand(Stop::Name, Stop::Description, botId)
        .add_localization(Russian::LocaleName, Russian::Stop::Name, Russian::Stop::Description));

    return commands;
}

Bot::Commands::Commands()
    : m_parsed(false)
{}

void Bot::Commands::parse(const dpp::slashcommand_map& commands)
{
    std::vector<dpp::slashcommand> registerCommands = GetCommands(0);
    if (commands.size() != registerCommands.size())
    {
        throw std::invalid_argument(fmt::format(
            "kb::Bot::Commands::parse(): Incorrect count of registered commands: {}/{}",
            commands.size(),
            registerCommands.size()
        ));
    }

    for (const auto& command : commands)
    {
        if (command.second.name == Help::Name)
            m_help = command.second;
        else if (command.second.name == Session::Name)
            m_session = command.second;
        else if (command.second.name == Settings::Name)
            m_settings = command.second;
        else if (command.second.name == Stats::Name)
            m_stats = command.second;
        else if (command.second.name == Set::Name)
            m_set = command.second;
        else if (command.second.name == Join::Name)
            m_join = command.second;
        else if (command.second.name == Leave::Name)
            m_leave = command.second;
        else if (command.second.name == Play::Name)
            m_play = command.second;
        else if (command.second.name == Pause::Name)
            m_pause = command.second;
        else if (command.second.name == Seek::Name)
            m_seek = command.second;
        else if (command.second.name == Shuffle::Name)
            m_shuffle = command.second;
        else if (command.second.name == Skip::Name)
            m_skip = command.second;
        else if (command.second.name == Clear::Name)
            m_clear = command.second;
        else if (command.second.name == Stop::Name)
            m_stop = command.second;
        else
        {
            throw std::invalid_argument(fmt::format(
                "Unknown command encountered: \"/{}\"",
                command.second.name
            ));
        }
    }
    m_parsed = true;
}

} // namespace kb
