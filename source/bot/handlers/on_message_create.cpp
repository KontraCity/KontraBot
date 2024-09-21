#include "bot/bot.hpp"

namespace kc {

void Bot::Bot::onMessageCreate(const dpp::message_create_t& event)
{
    if (event.msg.content.empty() || event.msg.content.find(fmt::format("<@{}>", me.id)) == std::string::npos)
        return;

    dpp::guild* guild = dpp::find_guild(event.msg.guild_id);
    m_logger.info(
        "\"{}\" / \"{}\": Replying to mention message: \"{}\"",
        guild->name, event.msg.author.format_username(), event.msg.content
    );
    event.reply(Info(guild->id).settings().locale->mention());
}

} // namespace kc
