#pragma once

namespace languages
{

namespace options
{
static int const kEnglish = 1;
static int const kSpanish = 2;
static int const kFrench = 3;
}

} 

namespace MainMenu
{
    namespace Options
    {
        static int const kStart = 1;
        static int const kQuit = 2;
        static int const kLanguage = 3;
    }

}

namespace Display
{
    static int const kBoardColWidth = 3; // for std::setw()
}

namespace BoardConfig
{
    namespace Limits
    {
        static int const kMinWidth = 24;
        static int const kMaxdWidth = 50;
        static int const kMinHeight = 24;
        static int const kMaxHeight = 50;
    }

}

namespace MineConfig
{
    namespace Limits
    {
        static int const kMin = 3;
        static int const kMax = 8;
    }
}

namespace PlayerCreation
{
    namespace Options
    {
        static constexpr char kStopCreation = '*';
        static constexpr char kHuman = 'H';
        static constexpr char kPC = 'P';
    }
}