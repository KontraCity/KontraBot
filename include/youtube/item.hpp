#pragma once

// STL modules
#include <variant>

// Custom modules
#include "youtube/video.hpp"
#include "youtube/playlist.hpp"

namespace kb {

namespace Youtube
{
    class Item : public std::variant<std::monostate, Video, Playlist>
    {
    public:
        enum class Type
        {
            None,
            Video,
            Playlist,
        };

    public:
        using variant::variant;

    public:
        /// @brief Get item type
        /// @return Item type
        inline Type type() const
        {
            return static_cast<Type>(index());
        }

        /// @brief Check if item not empty
        /// @return True if item not empty
        inline operator bool() const
        {
            return static_cast<Type>(index()) != Type::None;
        }
    };
}

} // namespace kb
