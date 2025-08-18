#pragma once

#include <vector>
#include <string>

template <typename T, typename TagT>
class StrongType
{
public:
    constexpr explicit StrongType(T value)
    : mValue{value}
    {
    }

    constexpr StrongType(StrongType const&) = default;
    constexpr StrongType(StrongType&&) = default;
    constexpr StrongType& operator=(StrongType const&) = default;
    constexpr StrongType& operator=(StrongType&&) = default;

    void setValue(T value)
    {
        mValue = value;
    }

    constexpr T const& raw() const
    {
        return mValue;
    }

private:
    T mValue;
};

typedef StrongType<unsigned int, struct WidthTag> Width;
typedef StrongType<unsigned int, struct HeightTag> Height;

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
    unsigned int remainingMines = 0;
    unsigned int remainingGuesses = 0;
    unsigned int opponentMinesDetected = 0;
    unsigned int ownMinesDetected = 0;
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
    unsigned int round = 1;
    unsigned int mines = 0;
    unsigned int initialMines = 0;
    Players players;
};

