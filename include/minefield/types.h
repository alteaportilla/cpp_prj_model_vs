#pragma once

#include <vector>
#include <string>
#include <unordered_map>

template <typename T, typename TagT>
class StrongType
{
public:
    constexpr explicit StrongType(T&& value)
    : mValue{value}
    {
    }

    constexpr StrongType(StrongType const&) = default;
    constexpr StrongType(StrongType&&) = default;
    constexpr StrongType& operator=(StrongType const&) = default;
    constexpr StrongType& operator=(StrongType&&) = default;

    constexpr T const& getValue() const
    {
        return mValue;
    }

    void setValue(T newValue)
    {
        this->mValue = newValue;
    }

private:
    T mValue;
};

typedef StrongType<unsigned int, struct WidthTag> Width;
typedef StrongType<unsigned int, struct HeightTag> Height;
typedef StrongType<unsigned int, struct RoundTag> Round;
typedef StrongType<unsigned int, struct MinesCountTag> MinesCount;
typedef StrongType<unsigned int, struct GuessesCountTag> GuessesCount;
typedef StrongType<unsigned int, struct DetectedMinesTag> DetectedMines;

enum class PositionState
{
    Empty,         // -> 0
    WithMine,      // -> 1 
    Removed,       // -> 2, if two mines are in the same position, they are both removed
    GuessedEmpty,  // -> 3, a player guessed a position that didn't have a mine
    GuessedMine    // -> 4, a player correctly guessed a mine
};

enum class PlayerType
{
    None,
    HumanPlayer,
    PC
};

struct MinePosition
{
    unsigned int x = 0;
    unsigned int y = 0;
    PositionState state = PositionState::Empty; 

    bool operator<(MinePosition const& other) const 
    {
        return (x < other.x) || (x == other.x && y < other.y);
    }

    bool operator==(MinePosition const& other) const 
    {
        return x == other.x && y == other.y;
    }
};

struct Player;
struct State;
struct GameContext;

typedef std::vector<std::vector<MinePosition>> Board;
typedef std::vector<Player> Players;
typedef State NextState;
typedef NextState (*StateUpdateFn)(GameContext&);
typedef void (*EnterMineFn)(GameContext&, Player&);
typedef std::unordered_map<std::string, std::string> Language;

// used in utils.cpp

typedef MinePosition(*EnterPosFn)(unsigned int, unsigned int, Player const&);
typedef MinePosition(*RandomPosFn)(Width, Height);

struct Player
{
    std::string name;
    PlayerType type = PlayerType::None;
    std::vector<MinePosition> placedMines;
    std::vector<MinePosition> placedGuesses;
    std::vector<MinePosition> minesHistory;
    std::vector<MinePosition> guessesHistory;
    MinesCount remainingMines{0};
    GuessesCount remainingGuesses{0};
    DetectedMines opponentMinesDetected{0};
    DetectedMines ownMinesDetected{0};
    EnterMineFn enterMine = nullptr;

    bool operator==(Player const &other) const
    {
        return (name == other.name);
    }
};

struct State
{
    StateUpdateFn updateFunction = nullptr;
};

struct GameContext
{
    State currentState;
    Width width{0};
    Height height{0};
    Board board;
    Round round{1};
    MinesCount mines{0};
    MinesCount initialMines{0};
    Players players;
    Language language;
};

