#pragma once

#include <mutex>
#include <string>

namespace kb {

class Config {
public:
    static constexpr const char* Filename = "config.json";

public:
    static void GenerateSampleFile();

private:
    std::mutex m_mutex;
    std::string m_error;
    std::string m_discordBotApiToken;
    bool m_youtubeAuthEnabled = false;
    bool m_proxyEnabled = false;
    std::string m_proxyUrl;

private:
    Config();

    static inline Config& Instance() {
        static Config instance;
        return instance;
    }

public:
    static inline const std::string& Error() {
        std::lock_guard lock(Instance().m_mutex);
        return Instance().m_error;
    }

    static inline const std::string& DiscordBotApiToken() {
        std::lock_guard lock(Instance().m_mutex);
        return Instance().m_discordBotApiToken;
    }

    static inline bool YoutubeAuthEnabled() {
        std::lock_guard lock(Instance().m_mutex);
        return Instance().m_youtubeAuthEnabled;
    }
    
    static inline bool ProxyEnabled() {
        std::lock_guard lock(Instance().m_mutex);
        return Instance().m_proxyEnabled;
    }

    static inline const std::string& ProxyUrl() {
        std::lock_guard lock(Instance().m_mutex);
        return Instance().m_proxyUrl;
    }
};

} // namespace kb
