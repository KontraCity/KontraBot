#pragma once

// STL modules
#include <vector>

// Library {fmt}
#include <fmt/format.h>
#include <fmt/color.h>

// Custom modules
#include "youtube/video.hpp"
using namespace kb;

namespace kbt {

namespace Video
{
    class TestCase
    {
    public:
        enum class Result
        {
            InvalidIdUrl,
            Normal,
            Livestream,
            Premiere,
            VideoPrivate,
            VideoBlocked,
            YoutubeError,
            Unknown
        };

    private:
        /// @brief Convert result to string
        /// @param result Result to convert
        /// @return Converted string
        static const char* ResultToString(Result result);

        /// @brief Get test result
        /// @param id ID of video to test
        /// @return Test result
        static Result GetResult(const std::string& id);

    private:
        const char* m_comment;
        const char* m_id;
        Result m_result;

    public:
        /// @brief Create a test case
        /// @param comment Test case comment
        /// @param id Test video ID
        /// @param result Expected test result
        TestCase(const char* comment, const char* id, Result result);

    public:
        /// @brief Perform the test
        /// @param number Test number
        /// @return True if test succeeded
        bool test(size_t number) const;

    public:
        /// @brief Get test case comment
        /// @return Test case comment
        inline const char* comment() const
        {
            return m_comment;
        }

        /// @brief Get test video ID
        /// @return Test video ID
        inline const char* id() const
        {
            return m_id;
        }

        /// @brief Get expected test result
        /// @return Expected test result
        inline Result result() const
        {
            return m_result;
        }
    };

    // YouTube video module test cases
    inline const std::vector<TestCase> TestCases = {
        // Incorrect video ID detection
        TestCase{ "Incorrect video ID detection #1", "", TestCase::Result::InvalidIdUrl },
        TestCase{ "Incorrect video ID detection #2", "abc", TestCase::Result::InvalidIdUrl },
        TestCase{ "Incorrect video ID detection #3", "abcdefghijklmnopqrstuvwxyz", TestCase::Result::InvalidIdUrl },
        TestCase{ "Incorrect video ID detection #4", "https://vid.plus/FlRa-iH7PGw", TestCase::Result::InvalidIdUrl },
        TestCase{ "Incorrect video ID detection #5", "https://invidio.us/watch?v=BaW_jenozKc", TestCase::Result::InvalidIdUrl },
        TestCase{ "Incorrect video ID detection #6", "https://redirect.invidious.io/watch?v=BaW_jenozKc", TestCase::Result::InvalidIdUrl },
        TestCase{ "Incorrect video ID detection #7", "https://vid.plus/FlRa-iH7PGw", TestCase::Result::InvalidIdUrl },
        TestCase{
            "Incorrect video ID detection #8",
            "https://zwearz.com/watch/9lWxNJF-ufM/electra-woman-dyna-girl-official-trailer-grace-helbig.html",
            TestCase::Result::InvalidIdUrl
        },

        // Correct video ID/URL detection
        TestCase{ "Correct video ID/URL detection #1", "BaW_jenozKc", TestCase::Result::Normal },
        TestCase{ "Correct video ID/URL detection #2", "https://www.youtube.com/watch?v=BaW_jenozKc", TestCase::Result::Normal },
        TestCase{ "Correct video ID/URL detection #3", "https://youtu.be/BaW_jenozKc?si=GJ0AMC", TestCase::Result::Normal },
        TestCase{ "Correct video ID/URL detection #4", "https://music.youtube.com/watch?v=MgNrAu2pzNs", TestCase::Result::Normal },
        TestCase{ "Correct video ID/URL detection #5", "https://www.youtube.com/watch?v=BaW_jenozKc&v=yZIXLfi8CZQ", TestCase::Result::Normal },
        TestCase{ "Correct video ID/URL detection #6", "https://www.youtube.com/embed/BaW_jenozKc", TestCase::Result::Normal },
        TestCase{
            "Correct video ID/URL detection #7",
            "https://www.youtube.com/watch?v=OPf0YbXqDm0&list=PLxA687tYuMWhkqYjvAGtW_heiEL4Hk_Lx&index=1",
            TestCase::Result::Normal
        },

        // Non-existent videos
        TestCase{ "Non-existent video #1", "abcdefghijk", TestCase::Result::YoutubeError },
        TestCase{ "Non-existent video #2", "AAAAAAAAAAA", TestCase::Result::YoutubeError },

        // Generic videos
        TestCase{ "Generic video #1", "BaW_jenozKc", TestCase::Result::Normal },
        TestCase{ "Generic video #2", "a9LDPn-MO4I", TestCase::Result::Normal },
        TestCase{ "Generic video #3", "xZLuqGNDoK0", TestCase::Result::Normal },

        // Music videos
        TestCase{ "Music video #1", "YTH8cxXBGBo", TestCase::Result::Normal },
        TestCase{ "Music video #2", "NPiAK33CfxQ", TestCase::Result::Normal },
        TestCase{ "Music video #3", "uDGJ9DyF25I", TestCase::Result::Normal },

        // Private videos
        TestCase{ "Private video #1", "V36LpHqtcDY", TestCase::Result::VideoPrivate },
        TestCase{ "Private video #2", "yZIXLfi8CZQ", TestCase::Result::VideoPrivate },

        // Blocked videos
        TestCase{ "Blocked video #1", "sJL6WA-aGkQ", TestCase::Result::VideoBlocked },

        // Age-gated videos
        TestCase{ "Age-gated video #1", "HtVdAasjOgU", TestCase::Result::Normal },
        TestCase{ "Age-gated video #2", "-PiKqkhyIXM", TestCase::Result::Normal },
        TestCase{ "Age-gated video #3", "Tq92D6wQ1mg", TestCase::Result::Normal },

        // Age-gated (non-bypassable for extraction) videos
        TestCase{ "Age-gated (non-bypassable for extraction) video #1", "Cr381pDsSsA", TestCase::Result::Normal },
    };

    /// @brief Test YouTube Video module
    /// @return Executable exit code
    int Test();
}

} // namespace kbt
