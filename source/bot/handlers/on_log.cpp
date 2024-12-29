#include "bot/bot.hpp"

// Custom modules
#include "common/utility.hpp"

namespace kb {

void Bot::Bot::onLog(const dpp::log_t& event, bool registerCommands)
{
    static spdlog::logger logger = Utility::CreateLogger("dpp");
    switch (event.severity)
    {
        case dpp::ll_warning:
        {
            /* Filtering needless warnings */
            if (event.message.find("You have attached an event to cluster::on_message_create()") != std::string::npos)
                break;
            if (registerCommands && event.message.find("Shard terminating due to cluster shutdown") != std::string::npos)
                break;
            if (event.message.find("Remote site requested reconnection") != std::string::npos)
                break;
            if (event.message.find("Terminating voice connection") != std::string::npos)
                break;
            if (event.message.find("Success") != std::string::npos)
                break;
            if (event.message.find("Received unhandled code") != std::string::npos)
                break;

            logger.warn(event.message);
            break;
        }
        case dpp::ll_error:
        {
            logger.error(event.message);
            break;
        }
        case dpp::ll_critical:
        {
            logger.critical(event.message);
            break;
        }
    }
}

} // namespace kb
