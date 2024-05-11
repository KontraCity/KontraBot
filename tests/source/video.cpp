#include "video.hpp"

namespace kct {

const char* Video::TestCase::ResultToString(Result result)
{
    switch (result)
    {
        case Video::TestCase::Result::InvalidIdUrl:
            return "InvalidIdUrl";
        case Video::TestCase::Result::Normal:
            return "Normal";
        case Video::TestCase::Result::Livestream:
            return "Livestream";
        case Video::TestCase::Result::Premiere:
            return "Premiere";
        case Video::TestCase::Result::VideoPrivate:
            return "VideoPrivate";
        case Video::TestCase::Result::VideoBlocked:
            return "VideoBlocked";
        case Video::TestCase::Result::YoutubeError:
            return "YoutubeError";
        default:
            return "Unknown";
    }
}

Video::TestCase::Result Video::TestCase::GetResult(const std::string& id)
{
    try
    {
        Youtube::Video video(id);
        switch (video.type())
        {
            case Youtube::Video::Type::Normal:
                return Video::TestCase::Result::Normal;
            case Youtube::Video::Type::Livestream:
                return Video::TestCase::Result::Livestream;
            case Youtube::Video::Type::Premiere:
                return Video::TestCase::Result::Premiere;
        }
    }
    catch (const std::invalid_argument&)
    {
        return Video::TestCase::Result::InvalidIdUrl;
    }
    catch (const Youtube::YoutubeError& error)
    {
        switch (error.type())
        {
            case Youtube::YoutubeError::Type::LoginRequired:
                return Video::TestCase::Result::VideoPrivate;
            case Youtube::YoutubeError::Type::Unplayable:
                return Video::TestCase::Result::VideoBlocked;
            case Youtube::YoutubeError::Type::YoutubeError:
                return Video::TestCase::Result::YoutubeError;
            default:
                break;
        }
    }
    return Video::TestCase::Result::Unknown;
}

Video::TestCase::TestCase(const char* comment, const char* id, Result result)
    : m_comment(comment)
    , m_id(id)
    , m_result(result)
{}

bool Video::TestCase::test(size_t number) const
{
    Result result = GetResult(m_id);
    if (result == m_result)
    {
        fmt::print(
            "{} Test case \"{}\" succeeded, result: \"{}\"\n",
            fmt::format(fmt::fg(fmt::color::green), "[#{:02}]", number),
            m_comment,
            ResultToString(result)
        );
        return true;
    }

    fmt::print(
        "{} Test case \"{}\" failed: expected \"{}\", got \"{}\"\n",
        fmt::format(fmt::fg(fmt::color::red), "[#{:02}]", number),
        m_comment,
        ResultToString(m_result),
        ResultToString(result)
    );
    return false;
}

int Video::Test()
{
    size_t casesFailed = 0;
    for (size_t index = 0, size = TestCases.size(); index < size; ++index)
    {
        bool succeeded = TestCases[index].test(index + 1);
        if (!succeeded)
            ++casesFailed;
    }

    if (!casesFailed)
    {
        fmt::print("YouTube Video module testing finished without fails\n");
        return 0;
    }

    fmt::print(
        "YouTube Video module testing finished with {} fail{}\n",
        casesFailed,
        casesFailed== 1 ? "" : "s"
    );
    return -1;
}

} // namespace kct
